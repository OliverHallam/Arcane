#include <stdio.h>
#include <cstdint>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

#include "libretro.h"

#include "Cart.h"
#include "Controller.h"
#include "Display.h"
#include "GameDatabase.h"
#include "NesSystem.h"
#include "RomFile.h"
#include <memory>

char retro_base_directory[4096];
char retro_game_path[4096];

static void fallback_log(enum retro_log_level level, const char* fmt, ...)
{
    (void)level;
    va_list va;
    va_start(va, fmt);
    vfprintf(stderr, fmt, va);
    va_end(va);
}


static retro_environment_t environ_cb;
static retro_video_refresh_t video_cb;
static retro_audio_sample_batch_t audio_batch_cb;
static retro_audio_sample_t audio_cb;
static retro_input_poll_t input_poll_cb;
static retro_input_state_t input_state_cb;

static std::unique_ptr<NesSystem> nesSystem;

void retro_init(void)
{
    nesSystem = std::make_unique<NesSystem>(44100);
}

void retro_deinit(void)
{
    nesSystem.reset();
}

unsigned retro_api_version(void)
{
    return RETRO_API_VERSION;
}

void retro_set_controller_port_device(unsigned port, unsigned device)
{
}

void retro_get_system_info(struct retro_system_info* info)
{
    memset(info, 0, sizeof(*info));
    info->library_name = "NesEmu";
    info->library_version = "0.1";
    info->need_fullpath = false;
    info->valid_extensions = "nes";
}

void retro_get_system_av_info(struct retro_system_av_info* info)
{
    info->geometry.base_width = Display::WIDTH;
    info->geometry.base_height = Display::HEIGHT;
    info->geometry.max_width = Display::WIDTH;
    info->geometry.max_height = Display::HEIGHT;
    info->geometry.aspect_ratio = 0.0;

    info->timing.fps = 60.0;
    info->timing.sample_rate = 44100.0;
}

void retro_set_environment(retro_environment_t cb)
{
    environ_cb = cb;

    struct retro_input_descriptor desc[] = {
        { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT,   "Left" },
        { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP,     "Up" },
        { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN,   "Down" },
        { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT,  "Right" },
        { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A,      "A" },
        { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B,      "B" },
        { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START,  "Start" },
        { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT, "Select" },
    };

    environ_cb(RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS, desc);

}

void retro_set_audio_sample(retro_audio_sample_t cb)
{
    audio_cb = cb;
}

void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb)
{
    audio_batch_cb = cb;
}

void retro_set_input_poll(retro_input_poll_t cb)
{
    input_poll_cb = cb;
}

void retro_set_input_state(retro_input_state_t cb)
{
    input_state_cb = cb;
}

void retro_set_video_refresh(retro_video_refresh_t cb)
{
    video_cb = cb;
}

static unsigned x_coord;
static unsigned y_coord;
static unsigned phase;
static int mouse_rel_x;
static int mouse_rel_y;

void retro_reset(void)
{
    x_coord = 0;
    y_coord = 0;
}

static void audio_callback(void)
{
}

static void audio_set_state(bool enable)
{
    (void)enable;
}

void retro_run(void)
{
    input_poll_cb();

    auto& controller = nesSystem->Controller1();

    uint32_t buttons = 0;

    if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT))
        buttons |= Buttons::Left;
    if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP))
        buttons |= Buttons::Up;
    if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN))
        buttons |= Buttons::Down;
    if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT))
        buttons |= Buttons::Right;
    if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A))
        buttons |= Buttons::A;
    if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B))
        buttons |= Buttons::B;
    if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START))
        buttons |= Buttons::Start;
    if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT))
        buttons |= Buttons::Select;

    controller.SetButtonState(buttons);

    nesSystem->RunFrame();

    const auto& display = nesSystem->Display();
    video_cb(display.Buffer(), Display::WIDTH, Display::HEIGHT, Display::WIDTH * sizeof(uint32_t));

    //static std::array<int16_t, Apu::SAMPLES_PER_FRAME * 2> audioFrame;
    //auto i = 0;
    //for (auto sample : nesSystem->Apu().Samples())
    //{
    //    audioFrame[i++] = sample;
    //    audioFrame[i++] = sample;
    //}

    //audio_batch_cb(&audioFrame[0], Apu::SAMPLES_PER_FRAME);

    //for (unsigned i = 0; i < 44100 / 60; i++, phase++)
    //{
    //    int16_t val = 0x800 * sinf(2.0f * 3.1415926535 * phase * 300.0f / 30000.0f);
    //    audio_cb(val, val);
    //}

    auto& apu = nesSystem->Apu();
    auto samplesPerFrame = apu.SamplesPerFrame();
    auto samples = apu.Samples();
    for (auto sampleIndex = 0u; sampleIndex < samplesPerFrame; sampleIndex++)
    {
        auto sample = samples[sampleIndex];
        audio_cb(sample, sample);
    }

}

bool retro_load_game(const struct retro_game_info* info)
{
    auto romFile = TryLoadINesFile(static_cast<const uint8_t*>(info->data), info->size);
    if (!romFile)
        return false;

    auto goodDescriptor = GameDatabase::Lookup(romFile->PrgData, romFile->ChrData);

    auto cart = TryCreateCart(
        goodDescriptor ? *goodDescriptor : romFile->Descriptor,
        std::move(romFile->PrgData),
        std::move(romFile->ChrData));

    if (!cart)
        return false;

    cart->Initialize();

    nesSystem->InsertCart(std::move(cart));
    nesSystem->Reset();

    auto fmt = retro_pixel_format::RETRO_PIXEL_FORMAT_XRGB8888;
    return environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &fmt);
}

void retro_unload_game(void)
{
    nesSystem->InsertCart(nullptr);
}

unsigned retro_get_region(void)
{
    return RETRO_REGION_NTSC;
}

bool retro_load_game_special(unsigned type, const struct retro_game_info* info, size_t num)
{
    return false;
}

size_t retro_serialize_size(void)
{
    return false;
}

bool retro_serialize(void* data_, size_t size)
{
    return false;
}

bool retro_unserialize(const void* data_, size_t size)
{
    return false;
}

void* retro_get_memory_data(unsigned id)
{
    (void)id;
    return NULL;
}

size_t retro_get_memory_size(unsigned id)
{
    (void)id;
    return 0;
}

void retro_cheat_reset(void)
{
}

void retro_cheat_set(unsigned index, bool enabled, const char* code)
{
    (void)index;
    (void)enabled;
    (void)code;
}
