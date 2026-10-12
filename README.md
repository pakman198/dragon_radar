# Dragon Radar

A Dragon Ball radar for the M5Stack StopWatch (round 466x466 AMOLED, ESP32-S3).
It shows a green grid screen with a red triangle at the center and yellow
dragon balls with their star counts, and plays a radar beep and a zoom sound.

Reference from the original show: https://www.youtube.com/watch?v=LxhUKnuE_h8

## Controls

| Input | Action |
| --- | --- |
| BtnA | Cycle zoom (500 / 1000 / 2000 / 4000 m) and play the zoom sound |
| BtnB | Mute / unmute |
| BtnA + BtnB held ~0.5 s | Power off |

Single presses act on release, so holding both buttons doesn't also zoom or mute.
Balls outside the current range are hidden, as on the show's radar.

## Files

- `dragon_radar.ino` - the sketch.
- `ping_sound.h` - one radar beep as 16-bit mono PCM at 16 kHz.
- `zoom_sound.h` - the zoom sound in the same format.

Keep all three files in the same folder, or the sketch won't compile.

## Setup

1. Install Arduino IDE.
2. Add the M5Stack board package (3.3.7 or later) and select the **M5StopWatch** board.
3. In Library Manager, install **M5Unified** (0.2.15 or later) and **M5GFX** (0.2.21 or later).
4. Open `dragon_radar.ino`, select the board and port, and click Upload.
5. If the upload can't connect, put the watch in download mode: hold the
   button for about 2 seconds until the green LED lights, then release.

Uploading replaces the factory firmware. To restore it, use M5Burner; see
<https://docs.m5stack.com/en/guide/restore_factory/stopwatch>.

## Customizing

- **Ball positions:** edit the `BALLS` list near the top of the sketch. Each
  row is the star count, the distance in meters, and the angle in radians
  (0 = right, positive = clockwise, `-PI/2` = straight up).
- **Beep rate:** it is set by the nearest ball's distance, from 150 ms (close)
  to 1200 ms (far).
- **Volume:** change the `setVolume` value in `setup()`.
- **Sounds:** the audio is a C array of PCM samples. To replace one, convert
  your clip to 16 kHz mono 16-bit WAV (for example with macOS `afconvert`) and
  regenerate the header.

## Notes

The beep and zoom sounds were cut from recordings of the original show. They are
copyrighted, so keep this project for personal use and don't redistribute the
audio.
