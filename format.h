#include <stdint.h>
#include <array>

typedef uint8_t u8;
typedef uint32_t u32;
typedef uint64_t u64;

/* ----------------------------------------------------------------------------
 * Pulled right from Extrems' definitions in the gba-as-gc multiboot code.
 * See https://github.com/ExtremsCorner/gba-as-controller */

struct buttons {
	uint16_t a          : 1;
	uint16_t b          : 1;
	uint16_t x          : 1;
	uint16_t y          : 1;

	uint16_t start      : 1;
	uint16_t get_origin : 1;
	uint16_t err_latch  : 1;
	uint16_t err_status : 1;

	uint16_t left       : 1;
	uint16_t right      : 1;
	uint16_t down       : 1;
	uint16_t up         : 1;

	uint16_t z          : 1;
	uint16_t r          : 1;
	uint16_t l          : 1;
	uint16_t use_origin : 1;
}__attribute__((packed));
struct {
	uint8_t type[2];

	struct {
		uint8_t mode   : 3;
		uint8_t motor  : 2;
		uint8_t origin : 1;
		uint8_t        : 2;
	} status;
} __attribute__((packed)) id;
struct status {
	struct buttons buttons;
	struct { uint8_t x, y; } stick;
	union {
		struct {
			struct { uint8_t x : 8, y : 8; } substick;
			struct { uint8_t r : 4, l : 4; } trigger;
			struct { uint8_t b : 4, a : 4; } button;
		} mode0;

		struct {
			struct { uint8_t y : 4, x : 4; } substick;
			struct { uint8_t l : 8, r : 8; } trigger;
			struct { uint8_t b : 4, a : 4; } button;
		} mode1;

		struct {
			struct { uint8_t y : 4, x : 4; } substick;
			struct { uint8_t r : 4, l : 4; } trigger;
			struct { uint8_t a : 8, b : 8; } button;
		} mode2;

		struct {
			struct { uint8_t x, y; } substick;
			struct { uint8_t l, r; } trigger;
		} mode3;

		struct {
			struct { uint8_t x, y; } substick;
			struct { uint8_t a, b; } button;
		} mode4;
	};
}__attribute__((packed));


/* ----------------------------------------------------------------------------
 * Pulled right from the Dolphin project.
 * See https://github.com/dolphin-emu/dolphin */

struct DTMHeader
{
  std::array<u8, 4> filetype;  // Unique Identifier (always "DTM"0x1A)

  std::array<char, 6> gameID;  // The Game ID
  bool bWii;                   // Wii game

  u8 controllers;  // Controllers plugged in (from least to most significant,
                   // the bits are GC controllers 1-4 and Wiimotes 1-4)

  bool bFromSaveState;  // false indicates that the recording started from bootup, true for savestate
  u64 frameCount;      // Number of frames in the recording
  u64 inputCount;      // Number of input frames in recording
  u64 lagCount;        // Number of lag frames in the recording
  u64 uniqueID;        // (not implemented) A Unique ID comprised of: md5(time + Game ID)
  u32 numRerecords;    // Number of rerecords/'cuts' of this TAS
  std::array<char, 32> author;  // Author's name (encoded in UTF-8)

  std::array<char, 16> videoBackend;   // UTF-8 representation of the video backend
  std::array<char, 16> audioEmulator;  // UTF-8 representation of the audio emulator
  std::array<u8, 16> md5;              // MD5 of game iso

  u64 recordingStartTime;  // seconds since 1970 that recording started (used for RTC)

  bool bSaveConfig;  // Loads the settings below on startup if true
  bool bSkipIdle;
  bool bDualCore;
  bool bProgressive;
  bool bDSPHLE;
  bool bFastDiscSpeed;
  u8 CPUCore;  // Uses the values of PowerPC::CPUCore
  bool bEFBAccessEnable;
  bool bEFBCopyEnable;
  bool bSkipEFBCopyToRam;
  bool bEFBCopyCacheEnable;
  bool bEFBEmulateFormatChanges;
  bool bImmediateXFB;
  bool bSkipXFBCopyToRam;
  u8 memcards;      // Memcards inserted (from least to most significant, the bits are slot A and B)
  bool bClearSave;  // Create a new memory card when playing back a movie if true
  u8 bongos;        // Bongos plugged in (from least to most significant, the bits are ports 1-4)
  bool bSyncGPU;
  bool bNetPlay;
  bool bPAL60;
  u8 language;
  std::array<u8, 11> reserved;      // Padding for any new config options
  std::array<char, 40> discChange;  // Name of iso file to switch to, for two disc games.
  std::array<u8, 20> revision;      // Git hash
  u32 DSPiromHash;
  u32 DSPcoefHash;
  u64 tickCount;                 // Number of ticks in the recording
  std::array<u8, 11> reserved2;  // Make heading 256 bytes, just because we can
}__attribute__((packed));

struct ControllerState
{
  bool Start : 1, A : 1, B : 1, X : 1, Y : 1, Z : 1;  // Binary buttons, 6 bits
  bool DPadUp : 1, DPadDown : 1,                      // Binary D-Pad buttons, 4 bits
      DPadLeft : 1, DPadRight : 1;
  bool L : 1, R : 1;      // Binary triggers, 2 bits
  bool disc : 1;          // Checks for disc being changed
  bool reset : 1;         // Console reset button
  bool is_connected : 1;  // Should controller be treated as connected
  bool reserved : 1;      // Reserved bits used for padding, 1 bit

  u8 TriggerL, TriggerR;          // Triggers, 16 bits
  u8 AnalogStickX, AnalogStickY;  // Main Stick, 16 bits
  u8 CStickX, CStickY;            // Sub-Stick, 16 bits
}__attribute__((packed));
