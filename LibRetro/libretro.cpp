#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

#include <stdio.h>
#if defined(_WIN32) && !defined(_XBOX)
#include <windows.h>
#endif
#include "libretro.h"

using namespace System;
using namespace System::IO;
using namespace System::Reflection;
using namespace System::Runtime::InteropServices;
using namespace NesEmu::Emulator;

#define VIDEO_WIDTH 256
#define VIDEO_HEIGHT 240
#define VIDEO_PIXELS VIDEO_WIDTH * VIDEO_HEIGHT

Assembly^ ResolveAssembly(Object^ sender, ResolveEventArgs^ args)
{
	auto assemblyName = gcnew AssemblyName(args->Name);
	if (assemblyName->Name == "Emulator")
	{
		auto path = Path::Combine(Path::GetDirectoryName(Assembly::GetExecutingAssembly()->Location), "Emulator.dll");
		return Assembly::LoadFile(path);
	}

	return nullptr;
}

ref class ManagedEmulator
{
public:
	static property EntertainmentSystem^ Instance
	{
		EntertainmentSystem^ get() { return instance; }
		void set(EntertainmentSystem^ value) { instance = value; }
	}

private:
	static EntertainmentSystem^ instance;
};


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
static retro_input_poll_t input_poll_cb;
static retro_input_state_t input_state_cb;


// the noinline is essential to make sure our AssemblyResolve handler hooks in before we try to use the emulator.
__declspec(noinline) void init_emulator()
{
	ManagedEmulator::Instance = gcnew NesEmu::Emulator::EntertainmentSystem();
}


void retro_init(void)
{
	System::AppDomain::CurrentDomain->AssemblyResolve += gcnew System::ResolveEventHandler(&ResolveAssembly);

	init_emulator();
}

void retro_deinit(void)
{
	ManagedEmulator::Instance = nullptr;

	System::AppDomain::CurrentDomain->AssemblyResolve -= gcnew System::ResolveEventHandler(&ResolveAssembly);
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
	info->geometry.base_width = VIDEO_WIDTH;
	info->geometry.base_height = VIDEO_HEIGHT;
	info->geometry.max_width = VIDEO_WIDTH;
	info->geometry.max_height = VIDEO_HEIGHT;
	info->geometry.aspect_ratio = 0.0;

	info->timing.fps = 60.0;
	info->timing.sample_rate = 0.0;
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
}

void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb)
{
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
	// TODO: input

	input_poll_cb();

	auto controller = ManagedEmulator::Instance->Controller;
	controller->Left   = input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT);
	controller->Up     = input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP);
	controller->Down   = input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN);
	controller->Right  = input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT);
	controller->A      = input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A);
	controller->B      = input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B);
	controller->Start  = input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START);
	controller->Select = input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT);

	ManagedEmulator::Instance->RunFrame();

	pin_ptr<uint32_t> buffer = &ManagedEmulator::Instance->Display->Frame[0];

	video_cb(buffer, 256, 240, 256 * sizeof(uint32_t));
}

bool retro_load_game(const struct retro_game_info* info)
{
	UnmanagedMemoryStream stream((unsigned char*)info->data, info->size);

	auto cart = Cart::TryLoad(%stream);
	if (cart == nullptr)
	{
		return false;
	}

	ManagedEmulator::Instance->InsertCart(cart);

	ManagedEmulator::Instance->Reset();

	enum retro_pixel_format fmt = RETRO_PIXEL_FORMAT_XRGB8888;
	return environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &fmt);
}

void retro_unload_game(void)
{
	ManagedEmulator::Instance->InsertCart(nullptr);
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
