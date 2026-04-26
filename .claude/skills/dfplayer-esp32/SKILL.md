# ESP32 DFPlayer Mini Control Skill

This skill provides guidance for developing, debugging, and optimizing the ESP32 DFPlayer Mini audio control system.

## Quick Reference

**Wiring (3 wires minimum):**
- ESP32 GPIO17 → DFPlayer RX (with 1kΩ resistor protection)
- ESP32 GPIO16 → DFPlayer TX
- Common GND

**UART Configuration:**
- Baud rate: 9600
- Bits: 8, Stop: 1, Parity: None
- Serial monitor: 115200

**Commands (via USB serial):**
```
play <n>          Play track n
folder <f> <n>    Play track n in folder f
vol <0-30>        Set volume (0-30)
pause/resume/stop Control playback
next/prev         Navigate tracks
status            Query current track
help              Show all commands
reset             Restart DFPlayer
```

## DFPlayer Protocol

**Frame format (10 bytes):**
```
0x7E 0xFF 0x06 CMD 0x00 PARAM_H PARAM_L CHECKSUM_H CHECKSUM_L 0xEF
```

**Checksum calculation:**
```
CHECKSUM = 0 - (0xFF + 0x06 + CMD + 0x00 + PARAM_H + PARAM_L)
```

**Common commands:**
| Command | Code | Purpose |
|---------|------|---------|
| Play track | 0x03 | `param = track_number` |
| Play folder | 0x0F | `param = (folder << 8) \| file` |
| Set volume | 0x06 | `param = volume (0-30)` |
| Pause | 0x0E | No parameter |
| Resume | 0x0D | No parameter |
| Stop | 0x16 | No parameter |
| Next | 0x01 | No parameter |
| Previous | 0x02 | No parameter |
| Query track | 0x42 | Response: bytes [5:7] = track |
| Reset | 0x0C | No parameter |

## microSD Card Setup

**Format:** FAT32 (essential)

**Directory structure (example):**
```
/mp3/
  0001.mp3
  0002.mp3
  0003.mp3
/folder1/
  0001.mp3
  0002.mp3
```

**Naming rules:**
- Filenames: 0001-9999 (must be 4 digits)
- Folders: 01-99 (DFPlayer uses folder numbers, not names)
- Format: MP3 (WAV, FLAC supported but test first)

## Development Workflow

### Build & Upload
```bash
pio run -e esp32dev -t upload
```

### Monitor Serial Output
```bash
pio device monitor -e esp32dev
# Baud: 115200 — matches setup() Serial.begin(115200)
```

### Debug Checklist
1. **Is the DFPlayer responding?**
   - Send `status` command
   - Monitor should show hex response: `7E FF 06 42 ...`
   - If silence: check wiring, 1kΩ resistor on TX→RX, 5V power

2. **No audio output?**
   - `vol 20` (set volume explicitly)
   - Check microSD card is formatted FAT32, has MP3s
   - Verify file structure matches expectations (folder01/0001.mp3, etc.)
   - Test with a known-good MP3

3. **Command not executing?**
   - Check baud mismatch (9600 ↔ 115200 serial monitor is a common culprit)
   - Verify `sendCommand` is being called (add debug prints)
   - Confirm `readResponse` timeout isn't too short (150ms is standard)

### Adding New Features

**New playback command:**
1. Define the DFPlayer command code (e.g., 0x03 for play)
2. Write a function: `void myFunction(uint16_t param) { sendCommand(0x03, param); }`
3. Wire it into `handleSerialCommand()` to accept user input
4. Test on the device before committing

**Responding to DFPlayer events:**
1. Extend `readResponse()` to detect the response type (opcode in byte [3])
2. Parse the payload (most responses have status in bytes [5:7])
3. Log or act on the result

## Common Issues & Fixes

| Symptom | Cause | Fix |
|---------|-------|-----|
| No serial output on monitor | Wrong baud in monitor or wrong code | Verify `Serial.begin(115200)` and monitor set to 115200 |
| DFPlayer not responding | Wiring or power | Check 5V stable, GND common, RX/TX connected, 1kΩ resistor on TX |
| Volume too low | Default 20 is medium | `vol 25` or `vol 30` to test max |
| MP3s not found | Card not FAT32 or wrong folder structure | Reformat card, use 4-digit filenames (0001.mp3), folders numbered 01-99 |
| Intermittent failures | Insufficient delay between commands | Increase `delay(50)` in `sendCommand()` to 100-150ms |
| Checksums wrong | Byte-order confusion | Verify: `CHECKSUM = 0 - (sum of [0xFF, 0x06, CMD, 0x00, PARAM_H, PARAM_L])` as signed 16-bit, then cast to uint8_t for high/low bytes |

## Testing a Fresh Build

After upload, expected sequence:
1. Monitor prints initialization messages
2. Send `help` → see command list
3. Send `vol 20` → "Volumen ajustado a 20"
4. Send `play 1` → audio plays (if MP3 on card)
5. Send `status` → "Pista actual: 1" (or similar)

If audio plays on step 4 but step 5 times out, the response-reading logic may need tuning (increase `readResponse` timeout or add retry logic).

## Performance & Optimization

- **Delay tuning:** 50ms is standard; reduce to 30ms if timing is tight, increase to 150ms if responses are dropped
- **Status polling:** Default 10s interval (`statusInterval`) can be adjusted per use case (shorter = more responsive but more serial traffic)
- **Memory:** `main.cpp` is ~4KB; plenty of room for additional commands or state tracking on the ESP32

## References

- **DFPlayer Mini datasheet:** Look for "DFPlayer Mini Manual" (English) for full command set and response codes
- **PlatformIO docs:** https://docs.platformio.org/
- **ESP32 Arduino docs:** https://docs.espressif.com/projects/arduino-esp32/
- **UART communication on ESP32:** Search "ESP32 HardwareSerial" for pin mapping and initialization
