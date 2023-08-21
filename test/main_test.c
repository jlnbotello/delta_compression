#include "minut.h"
#include "compressor.h"
#include "read_csv.h"

#define DATA_LEN 10
#define BUF_LEN (DATA_LEN * 5)
#define B0(x) ((x >> 0*7) & 0x7F)
#define B1(x) ((x >> 1*7) & 0x7F)
#define B2(x) ((x >> 2*7) & 0x7F)
#define B3(x) ((x >> 3*7) & 0x7F)
#define B4(x) ((x >> 4*7) & 0x7F)

const int32_t original[DATA_LEN] = {-100, -10, 0, 20, 200, 101, 102, 103, 104, 105};
const int32_t delta[DATA_LEN] = {-100, 90, 10, 20, 180, -99, 1, 1, 1, 1};
//                                       2*100-1, 2*90, 2*10, 2*20, 2*180, 2*99-1, 2, 2, 2, 2
const uint32_t delta_zigzag[DATA_LEN] = {199, 180, 20, 40, 360, 197, 2, 2, 2, 2};
const uint8_t varint[] = {  B0(199)|0x80, B1(199), B0(180)|0x80, B1(180), B0(20), B0(40),
                            B0(360)|0x80, B1(360), B0(197)|0x80, B1(197), B0(2), B0(2), B0(2), B0(2)};
const size_t varint_len = sizeof(varint);

// Reset on each test --> call init_test() first
size_t out_len = 0;
size_t aux_u32_len = 0;
size_t aux_u8_len = 0;
size_t enc_len = 0;
size_t dec_len = 0;
size_t din_len = 0;

int32_t out_i32[BUF_LEN];
uint32_t out_u32[BUF_LEN];
uint8_t out_u8[BUF_LEN];

int32_t aux_i32[DATA_LEN];
uint32_t aux_u32[DATA_LEN];
uint8_t aux_u8[DATA_LEN];

static void init_test();

static bool equal_i32_buffers(const int32_t a[], const int32_t b[], size_t len);
static bool equal_u32_buffers(const uint32_t a[], const uint32_t b[], size_t len);
static bool equal_u8_buffers(const uint8_t a[], const uint8_t b[], size_t len);

static void print_i32_buffer(const char * name, const int32_t b[], size_t len);
static void print_u32_buffer(const char * name, const uint32_t b[], size_t len);
static void print_u8_buffer(const char * name, const uint8_t b[], size_t len);

static void check_retcode(RetCode retcode);

static void allocate2DArray(int32_t ***array, uint32_t rows, uint32_t cols);
static void release2DArray(int32_t **array, uint32_t rows);

int main(void)
{
    MINUT(1);
    return 0;
}

// Test definitions
TEST(test_delta_enc)
{
    init_test();

    check_retcode(delta_encoding(original, out_i32, DATA_LEN));

    ASSERT_EQ(true, equal_i32_buffers(out_i32, delta, DATA_LEN));
}

TEST(test_delta_dec)
{
    init_test();

    check_retcode(delta_decoding(delta, out_i32, DATA_LEN));

    ASSERT_EQ(true, equal_i32_buffers(out_i32, original, DATA_LEN));
}

TEST(test_delta_enc_dec)
{
    init_test();

    check_retcode(delta_encoding(original, aux_i32, DATA_LEN));
    check_retcode(delta_decoding(aux_i32, out_i32, DATA_LEN));

    ASSERT_EQ(true, equal_i32_buffers(out_i32, original, DATA_LEN));
}

TEST(test_zigzag_enc)
{
    init_test();

    check_retcode(zigzag_encoding(delta, out_u32, DATA_LEN));

    ASSERT_EQ(true, equal_i32_buffers(out_u32, delta_zigzag, DATA_LEN));
}

TEST(test_zigzag_dec)
{
    init_test();

    check_retcode(zigzag_decoding(delta_zigzag, out_u32, DATA_LEN));

    ASSERT_EQ(true, equal_i32_buffers(out_u32, delta, DATA_LEN));
}

TEST(test_zigzag_enc_dec)
{
    init_test();

    check_retcode(zigzag_encoding(delta, aux_u32, DATA_LEN));
    check_retcode(zigzag_decoding(aux_u32, out_i32, DATA_LEN));

    ASSERT_EQ(true, equal_i32_buffers(out_i32, delta, DATA_LEN));
}

TEST(test_varint_enc)
{
    init_test();

    check_retcode(varuint_encoding(delta_zigzag, DATA_LEN, out_u8, &out_len));

    ASSERT_EQ(true, (out_len == varint_len) &&
                     equal_u8_buffers(out_u8, varint, out_len));
}

TEST(test_varint_dec)
{
    init_test();

    check_retcode(varuint_decoding(varint, varint_len, out_u32, &out_len));

    ASSERT_EQ(true, (out_len == DATA_LEN) &&
                     equal_u32_buffers(out_u32, delta_zigzag, out_len));
}

TEST(test_varint_enc_dec)
{
    init_test();

    check_retcode(varuint_encoding(delta_zigzag, DATA_LEN, aux_u8, &aux_u8_len));
    check_retcode(varuint_decoding(aux_u8, aux_u8_len, out_u32, &out_len));

    ASSERT_EQ(true, (out_len == DATA_LEN) &&
                     equal_u32_buffers(delta_zigzag, out_u32, out_len));
}

TEST(test_full_enc_dec)
{
    init_test();

    check_retcode(delta_encoding(original, aux_i32, DATA_LEN));
    check_retcode(zigzag_encoding(aux_i32, aux_u32, DATA_LEN));
    check_retcode(varuint_encoding(aux_u32, DATA_LEN, aux_u8, &aux_u8_len));
    check_retcode(varuint_decoding(aux_u8, aux_u8_len, aux_u32, &aux_u32_len));
    check_retcode(zigzag_decoding(aux_u32, aux_i32, aux_u32_len));
    check_retcode(delta_decoding(aux_i32, out_i32, aux_u32_len));

    ASSERT_EQ(true, equal_i32_buffers(out_i32, original, aux_u32_len));
}

void copy_column(uint32_t col, int32_t ** src, int32_t * dst, size_t len)
{
    for (size_t i = 0; i < len; i++)
    {
        dst[i] = src[i][col];
    }    
}

TEST(test_signal_compression)
{
    init_test();

    #define MAX_ROW 5000
    #define MAX_COL 1

    uint32_t rows = MAX_ROW;
    uint32_t cols = MAX_COL;        
    int32_t din_i32[MAX_ROW];
    int32_t dout_i32[MAX_ROW];
    uint8_t buf_u8[MAX_ROW * 5];
    int32_t buf_i32[MAX_ROW];
    uint32_t buf_u32[MAX_ROW];

    // Import data from CSV
    int32_t **mat_i32;
    allocate2DArray(&mat_i32, MAX_ROW, MAX_COL);
    readCSV("../data/MLII_10s_360Hz.csv", mat_i32, &rows, &cols, true);
    din_len = rows;
    copy_column(0, mat_i32, din_i32, din_len);
    release2DArray(mat_i32, MAX_ROW);    

    // Compress
    check_retcode(delta_encoding(din_i32, buf_i32, din_len));
    check_retcode(zigzag_encoding(buf_i32, buf_u32, din_len));
    check_retcode(varuint_encoding(buf_u32, din_len, buf_u8, &enc_len));
    
    // Decompress
    check_retcode(varuint_decoding(buf_u8, enc_len, buf_u32, &dec_len));
    check_retcode(zigzag_decoding(buf_u32, buf_i32, dec_len));
    check_retcode(delta_decoding(buf_i32, dout_i32, dec_len));

    // Results
    uint32_t unc_len = din_len * sizeof(int32_t);
    uint32_t com_len = enc_len * sizeof(uint8_t);
    float com_ratio =  (float)  unc_len / com_len;

    printf("info test_signal_compression\n");
    printf("  Uncompressed length (bytes): %d\n", unc_len) ;
    printf("  Compressed length (bytes): %d\n", com_len);   
    printf("  Compression ratio: %.1f\n", com_ratio);

    ASSERT_EQ(true, equal_i32_buffers(dout_i32, din_i32, din_len));
}

// Declare tests to be run:
MINUT_BEG
RUN(test_delta_enc());
RUN(test_delta_dec());
RUN(test_delta_enc_dec());

RUN(test_zigzag_enc());
RUN(test_zigzag_dec());
RUN(test_zigzag_enc_dec());

RUN(test_varint_enc());
RUN(test_varint_dec());
RUN(test_varint_enc_dec());

RUN(test_full_enc_dec());
RUN(test_signal_compression());
MINUT_END

// Helpers
void init_test()
{
    out_len = 0;
    aux_u32_len = 0;
    aux_u8_len = 0;
    enc_len = 0;
    dec_len = 0;
    din_len = 0;

    for (size_t i = 0; i < BUF_LEN; i++)
    {
        out_i32[i] = 0;
        out_u32[i] = 0;
        out_u8[i] = 0;
    }
}

bool equal_i32_buffers(const int32_t a[], const int32_t b[], size_t len)
{
    for (size_t i = 0; i < len; i++)
    {
        if (a[i] != b[i])
        {
            print_i32_buffer("a", a, len);
            print_i32_buffer("b", b, len);
            return false;
        }
    }

    return true;
}

bool equal_u32_buffers(const uint32_t a[], const uint32_t b[], size_t len)
{
    for (size_t i = 0; i < len; i++)
    {
        if (a[i] != b[i])
        {
            print_u32_buffer("a", a, len);
            print_u32_buffer("b", b, len);
            return false;
        }
    }

    return true;
}

static bool equal_u8_buffers(const uint8_t a[], const uint8_t b[], size_t len)
{
    for (size_t i = 0; i < len; i++)
    {
        if (a[i] != b[i])
        {
            print_u8_buffer("a", a, len);
            print_u8_buffer("b", b, len);
            return false;
        }
    }

    return true;
}

void print_i32_buffer(const char * name, const int32_t b[], size_t len)
{
    printf("%s:[%d", name, b[0]);
    for (size_t i = 1; i < len; i++)
    {
        printf(", %d", b[i]);
    }
    printf("](%d)\n", len);
}

void print_u32_buffer(const char * name, const uint32_t b[], size_t len)
{
    printf("%s:[%d", name, b[0]);
    for (size_t i = 1; i < len; i++)
    {
        printf(", %d", b[i]);
    }
    printf("](%d)\n", len);
}

void print_u8_buffer(const char * name, const uint8_t b[], size_t len)
{
    printf("%s:[%d", name, b[0]);
    for (size_t i = 1; i < len; i++)
    {
        printf(", %d", b[i]);
    }
    printf("](%d)\n", len);
}

void check_retcode(RetCode retcode)
{
    if (retcode != SUCCESS)
    {
        printf("Error: %d", retcode);
    }
}

void allocate2DArray(int32_t ***array, uint32_t rows, uint32_t cols) {
    *array = (int32_t **)malloc(rows * sizeof(int32_t *));
    for (uint32_t i = 0; i < rows; i++) {
        (*array)[i] = (int32_t *)malloc(cols * sizeof(int32_t));
    }
}

void release2DArray(int32_t **array, uint32_t rows) {
    for (uint32_t i = 0; i < rows; i++) {
        free(array[i]);
    }
    free(array);
}