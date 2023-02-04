#include "../log/log.h"

#include <iostream>
#include <fcntl.h>
#include <cassert>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>

constexpr uint8_t NALU_NRI_MASK = 0x60;
constexpr uint8_t NALU_F_NRI_MASK = 0xe0;
constexpr uint8_t NALU_TYPE_MASK = 0x1F;

constexpr uint8_t FU_S_MASK = 0x80;
constexpr uint8_t FU_E_MASK = 0x40;
constexpr uint8_t SET_FU_A_MASK = 0x1C;

class H264Parser
{
private:
        int fd = -1;
        int64_t file_size = 0;
        uint8_t *ptr_mapped_file_cur = nullptr;
        uint8_t *ptr_mapped_file_start = nullptr, *ptr_mapped_file_end = nullptr;
        static const uint8_t *find_next_start_code(const uint8_t *_buffer, const int64_t _bufLen);
        
public:
    explicit H264Parser(const char* filepath);
    ~H264Parser();
    static bool is_start_code(const uint8_t *_buffer, int64_t buffer_len, uint8_t start_code_type);
    std::pair<const uint8_t *, int64_t> get_next_frame();
};