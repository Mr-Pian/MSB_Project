//
// Created by pickaxehit on 24-9-27.
//

#include "ssd1306.h"
#include "font_12_24.h"
#include "font_5_7.h"
#include "i2c.h"

#include <stdio.h>
#include <string.h>

// uint8_t bus_busy = 0;

HAL_StatusTypeDef i2c_mem_write(I2C_HandleTypeDef *hi2c, uint16_t dev_address, uint16_t target_mem_address,
                                uint16_t target_mem_size, uint8_t *data, uint16_t size) {
    // 使用阻塞式函数，等待传输完成，超时时间可调
    return HAL_I2C_Mem_Write(hi2c, dev_address, target_mem_address, target_mem_size, data, size, HAL_MAX_DELAY);
}

HAL_StatusTypeDef i2c_mem_read(I2C_HandleTypeDef *hi2c, uint16_t dev_address, uint16_t target_mem_address,
                               uint16_t target_mem_size, uint8_t *data, uint16_t size) {
    // 使用阻塞式函数
    return HAL_I2C_Mem_Read(hi2c, dev_address, target_mem_address, target_mem_size, data, size, HAL_MAX_DELAY);
}

// void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c) {
//     if (hi2c->Instance == I2C1) {
//         bus_busy = 0;
//     }
// }
//
// void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *hi2c) {
//     if (hi2c->Instance == I2C1) {
//         bus_busy = 0;
//     }
// }

uint8_t ssd1306_buffer[512];

const uint8_t ssd1306_init_command[] = {0xAE, 0x00, 0x10, 0x40, 0x81, 0xCF, 0xA0, 0xC0, 0xA6, 0xA8,
                                        0x1F, 0xD3, 0x00, 0xD5, 0x80, 0xD9, 0xF1, 0xDA, 0x02, 0xDB,
                                        0x40, 0x20, 0x00, 0x8D, 0x14, 0xA4, 0xA6, 0xAF};

void ssd1306_init(void) {
    i2c_mem_write(&hi2c1, 0x78, 0x00, I2C_MEMADD_SIZE_8BIT, ssd1306_init_command, sizeof(ssd1306_init_command));
    HAL_Delay(10);
}

void ssd1306_refresh(void) {
    i2c_mem_write(&hi2c1, 0x78, 0x40, I2C_MEMADD_SIZE_8BIT, ssd1306_buffer, sizeof(ssd1306_buffer));
}

void ssd1306_clear(uint8_t color) {
    if (color) {
        memset(ssd1306_buffer, 0xff, sizeof(ssd1306_buffer));
    } else {
        memset(ssd1306_buffer, 0x00, sizeof(ssd1306_buffer));
    }
}

static void ssd1306_char_5_7_mem_cp(uint8_t x, uint8_t y, char c, uint8_t color, uint8_t i) {
    if (y >= 32)
        return;

    if (color == 0) {
        if (y % 8 != 0) {
            ssd1306_buffer[(y - y % 8) / 8 * 128 + x + i] &= ~(font_5_7_data[(c - 32) * 5 + i] << y % 8);
            if (((y - y % 8) / 8 + 1) * 128 < 512) {
                ssd1306_buffer[((y - y % 8) / 8 + 1) * 128 + x + i] &=
                    ~(font_5_7_data[(c - 32) * 5 + i] >> (8 - y % 8));
            }
        } else {
            ssd1306_buffer[y / 8 * 128 + x + i] &= ~(font_5_7_data[(c - 32) * 5 + i]);
        }
    } else {
        if (y % 8 != 0) {
            ssd1306_buffer[(y - y % 8) / 8 * 128 + x + i] |= (font_5_7_data[(c - 32) * 5 + i] << y % 8);
            if (((y - y % 8) / 8 + 1) * 128 < 512) {
                ssd1306_buffer[((y - y % 8) / 8 + 1) * 128 + x + i] |= (font_5_7_data[(c - 32) * 5 + i] >> (8 - y % 8));
            }
        } else {
            ssd1306_buffer[y / 8 * 128 + x + i] |= (font_5_7_data[(c - 32) * 5 + i]);
        }
    }
}

void ssd1306_draw_char_5_7(uint8_t x, uint8_t y, char c, uint8_t color) {
    if (y >= 32)
        return;
    if (x > 123) {
        for (uint8_t i = 0; i < 128 - x; i++) {
            ssd1306_char_5_7_mem_cp(x, y, c, color, i);
        }
        return;
    }
    for (uint8_t i = 0; i < 5; i++) {
        ssd1306_char_5_7_mem_cp(x, y, c, color, i);
    }
}

void ssd1306_draw_string_5_7(uint8_t x, uint8_t y, const char *s, uint8_t color, uint8_t boundary) {
    size_t len = strlen(s);
    uint8_t px = x;
    uint8_t py = y;
    for (size_t i = 0; i < len; i++) {
        if (py >= 32)
            return;
        if (s[i] == '\n') {
            px = 0;
            py += 8;
            continue;
        }
        ssd1306_draw_char_5_7(px, py, s[i], color);
        px += 6;
        if (px > 123) {
            if (!boundary) {
                size_t j = i;
                for (; i < len; i++) {
                    if (s[i] == '\n') {
                        ssd1306_draw_char_5_7(px, py, s[j + 1], color);
                        px = 0;
                        py += 8;
                        break;
                    }
                }
                if (s[i] == '\n') {
                    continue;
                }
                ssd1306_draw_char_5_7(px, py, s[j + 1], color);
                return;
            }
            px = 0;
            py += 8;
        }
    }
}

static void ssd1306_char_12_24_mem_cp(uint8_t x, uint8_t y, const uint8_t *font_col_data, uint8_t color, uint8_t i) {
    if (y + 24 > 32)
        return;// 超出底部边界

    uint8_t start_page = y / 8;// 起始 Page (0-3)
    uint8_t y_offset = y % 8;  // Page 内的垂直位移 (0-7)
    uint8_t current_x = x + i;

    // 组合 3 个字节为 24 位数据
    uint32_t char_col_data = 0;
    char_col_data |= (uint32_t) font_col_data[0] << 0;
    char_col_data |= (uint32_t) font_col_data[1] << 8;
    char_col_data |= (uint32_t) font_col_data[2] << 16;
    // 执行垂直位移：将数据向上移动 y_offset 像素
    // 结果是 32 位，可能跨越 4 个 Page
    uint32_t shifted_data = char_col_data << y_offset;

    // 遍历写入 4 个 Page
    for (int p = 0; p < 4; p++) {
        uint8_t current_page = start_page + p;
        if (current_page >= 4)
            break;

        // 获取当前 Page 对应的 8 位数据
        uint8_t byte_val = (uint8_t) ((shifted_data >> (p * 8)) & 0xFF);

        if (byte_val == 0)
            continue;// 优化：如果当前 Page 没有数据，则跳过

        uint16_t buff_idx = current_page * 128 + current_x;

        if (color) {// WHITE (1) - 逻辑或操作 (设置位)
            ssd1306_buffer[buff_idx] |= byte_val;
        } else {// BLACK (0) - 逻辑与非操作 (清除位)
            ssd1306_buffer[buff_idx] &= ~byte_val;
        }
    }
}

void ssd1306_draw_char_12_24(uint8_t x, uint8_t y, char c, uint8_t color) {
    const uint8_t char_width = 12;  // 12
    const uint8_t bytes_per_col = 3;// 3
                                    // 1. 边界检查
    if (y + 24 > 32 || x >= 128) {
        return;
    }

    // 2. 字体数据索引计算
    if (c < 32 || c > 126) {
        c = 32;// 默认使用空格
    }
    uint16_t char_index = c - 32;
    uint16_t char_offset = char_index * 36;

    // 3. 遍历字符的每一列 (共 12 列)
    for (uint8_t i = 0; i < char_width; i++) {
        uint8_t current_x = x + i;
        if (current_x >= 128)
            break;

        const uint8_t *data_ptr = &font_12_24_data[char_offset + i * bytes_per_col];

        // 绘制当前列
        ssd1306_char_12_24_mem_cp(x, y, data_ptr, color, i);
    }
}

void ssd1306_draw_string_12_24(uint8_t x, uint8_t y, const char *s, uint8_t color, uint8_t boundary) {
    const uint8_t char_width = 12;                  // 12
    const uint8_t next_char_x_step = char_width + 1;// 12 + 1 像素间距
    const uint8_t next_line_y_step = 8;
    const uint8_t max_x = 128 - next_char_x_step;// 确保下一个字符能完整显示

    uint8_t px = x;
    uint8_t py = y;

    // 假设您不需要 5x7 字符串函数中复杂的截断逻辑 (boundary == 0)
    // 采用更简洁的实现：超出边界则换行（boundary=1），或停止绘制（boundary=0）。
    // 您的 5x7 实现中 boundary=0 似乎是为了处理单词截断，但 12x24 字体通常用于固定字符集的行显示，
    // 我们在此简化为超出边界即换行（如果允许），或停止。

    while (*s) {
        if (py + 24 > 32) {
            // 超出底部边界，停止绘制
            return;
        }

        if (*s == '\n') {
            px = 0;
            py += next_line_y_step;
            s++;
            continue;
        }

        // 检查是否需要换行
        if (px > max_x) {
            if (boundary) {
                // 自动换行
                px = 0;
                py += next_line_y_step;

                // 再次检查新行是否超出底部
                if (py + 24 > 32) {
                    return;
                }
            } else {
                // 不允许自动换行，停止绘制
                return;
            }
        }

        // 绘制字符
        ssd1306_draw_char_12_24(px, py, *s, color);

        // 移动到下一个字符位置
        px += next_char_x_step;
        s++;
    }
}

__weak void unifont_get_glyphs(uint32_t code, uint8_t *buffer, unifont_prop_t *prop) {
    return;
}

uint8_t ssd1306_draw_char_unifont(uint8_t x, uint8_t y, uint32_t code, uint8_t color) {
    if (y >= 16)
        return x;

    uint8_t unifont_buf[32];
    unifont_prop_t prop;
    unifont_get_glyphs(code, unifont_buf, &prop);

    if (prop.comb) {
        x -= prop.comb_off;
    }

    if (color == 0) {
        for (uint8_t i = 0; i < 16; i++, y++) {
            uint8_t buf_pos = i << prop.width;
            uint8_t page_y = y / 8;
            for (int j = 8 - 1; j >= 0; --j) {
                if (x + 8 - j > 128) {
                    continue;
                }
                ssd1306_buffer[page_y * 128 + x + 8 - j - 1] &= ~((unifont_buf[buf_pos] >> j & 0x01) << y % 8);
                if (prop.width) {
                    if (x + 16 - j > 128) {
                        continue;
                    }
                    ssd1306_buffer[page_y * 128 + x - j + 16 - 1] &= ~((unifont_buf[buf_pos + 1] >> j & 0x01) << y % 8);
                }
            }
        }
    } else {
        for (uint8_t i = 0; i < 16; i++, y++) {
            uint8_t buf_pos = i << prop.width;
            uint8_t page_y = y / 8;
            for (int j = 8 - 1; j >= 0; --j) {
                if (x + 8 - j > 128) {
                    continue;
                }
                ssd1306_buffer[page_y * 128 + x + 8 - j - 1] |= (unifont_buf[buf_pos] >> j & 0x01) << y % 8;
                if (prop.width) {
                    if (x + 16 - j > 128) {
                        continue;
                    }
                    ssd1306_buffer[page_y * 128 + x - j + 16 - 1] |= (unifont_buf[buf_pos + 1] >> j & 0x01) << y % 8;
                }
            }
        }
    }
    if (prop.comb) {
        return x + prop.comb_off;
    }
    if (prop.width) {
        return x + 16;
    }
    return x + 8;
}

int ssd1306_printf(uint8_t x, uint8_t y, uint8_t color, uint8_t boundary, const char *fmt, ...) {
    char str[1024];
    __builtin_va_list args;
    __builtin_va_start(args, fmt);
    int ret = 0;
    ret = vsnprintf(str, sizeof(str), fmt, args);
    ssd1306_draw_string_5_7(x, y, str, color, boundary);
    return ret;
}
void ssd1306_clear_page(uint8_t page_num, uint8_t color) {
    if (page_num >= 4) {
        return;// 边界检查
    }

    // 计算 Page 在缓冲区中的起始索引
    uint16_t start_index = page_num * 128;

    // 要填充的值：0x00 表示清空 (黑色)，0xFF 表示填充 (白色)
    uint8_t fill_value = (color == 0) ? 0x00 : 0xFF;

    // 使用 memset 清除 Page 对应的 128 个字节
    memset(&ssd1306_buffer[start_index], fill_value, 128);
}
