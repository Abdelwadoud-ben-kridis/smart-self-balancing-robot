#!/usr/bin/env python3
"""SBR-01 Raspberry Pi 3.5" TFT display — boot, face, and telemetry dashboard."""

import argparse
import math
import os
import sys
import threading
import time
from collections import deque

import pygame

# ---------------------------------------------------------------------------
# Constants
# ---------------------------------------------------------------------------
W, H = 480, 320
FPS = 30

BG = (10, 12, 16)
CYAN = (77, 212, 232)
GREEN = (135, 169, 107)
ORANGE = (217, 119, 87)
YELLOW = (245, 217, 73)
ROSE = (200, 112, 112)
WHITE = (224, 224, 224)
DIM = (100, 100, 110)
MUTE = (60, 60, 68)
LINE = (30, 32, 40)
LINE2 = (40, 42, 50)

SERIAL_PORT = "/dev/serial0"
BAUD = 115200
TELE_HZ = 20
HISTORY = 200

# ---------------------------------------------------------------------------
# Telemetry
# ---------------------------------------------------------------------------
class Telemetry:
    def __init__(self):
        self.pitch = 0.0
        self.motor_l = 0
        self.motor_r = 0
        self.state = "SIT"
        self.last_rx = 0.0
        self.lock = threading.Lock()
        self.history = deque(maxlen=HISTORY)
        self.rx_count = 0
        self.start_time = time.time()

    def update(self, pitch, ml, mr, state):
        with self.lock:
            self.pitch = pitch
            self.motor_l = ml
            self.motor_r = mr
            self.state = state
            self.last_rx = time.time()
            self.history.append(pitch)
            self.rx_count += 1

    def snapshot(self):
        with self.lock:
            return {
                "pitch": self.pitch,
                "motor_l": self.motor_l,
                "motor_r": self.motor_r,
                "state": self.state,
                "last_rx": self.last_rx,
                "history": list(self.history),
                "connected": (time.time() - self.last_rx) < 2.0 if self.last_rx else False,
                "uptime": time.time() - self.start_time,
                "rx_count": self.rx_count,
            }


def serial_reader(tele, port, baud):
    import serial
    while True:
        try:
            ser = serial.Serial(port, baud, timeout=0.5)
            while True:
                raw = ser.readline()
                if not raw:
                    continue
                line = raw.decode("ascii", errors="ignore").strip()
                if not line.startswith("T:"):
                    continue
                parts = line[2:].split(",")
                if len(parts) != 4:
                    continue
                try:
                    tele.update(float(parts[0]), int(float(parts[1])),
                                int(float(parts[2])), parts[3].strip())
                except ValueError:
                    pass
        except Exception:
            time.sleep(1)


def demo_generator(tele):
    states = ["SIT", "STD", "BAL"]
    t0 = time.time()
    while True:
        t = time.time() - t0
        cycle = int(t / 8) % 3
        state = states[cycle]
        if state == "SIT":
            pitch = 0.5 * math.sin(t * 0.4)
        elif state == "STD":
            pitch = 3.0 * math.sin(t * 1.2)
        else:
            pitch = 8.0 * math.sin(t * 2.5) + 2.0 * math.sin(t * 0.3)
        ml = int(pitch * 18 + 30 * math.sin(t * 3))
        mr = int(pitch * 18 - 30 * math.sin(t * 3))
        tele.update(pitch, max(-255, min(255, ml)), max(-255, min(255, mr)), state)
        time.sleep(1.0 / TELE_HZ)


# ---------------------------------------------------------------------------
# Drawing helpers
# ---------------------------------------------------------------------------
def lerp_color(a, b, t):
    t = max(0.0, min(1.0, t))
    return tuple(int(a[i] + (b[i] - a[i]) * t) for i in range(3))


def draw_gradient_bar(surf, rect, c1, c2, pct):
    x, y, w, h = rect
    pygame.draw.rect(surf, LINE, (x, y, w, h), border_radius=1)
    fill_w = int(w * max(0.0, min(1.0, pct)))
    if fill_w < 2:
        return
    for i in range(fill_w):
        col = lerp_color(c1, c2, i / max(1, w))
        pygame.draw.line(surf, col, (x + i, y), (x + i, y + h - 1))


def pitch_color(p):
    a = abs(p)
    if a < 10:
        return GREEN
    if a < 30:
        return YELLOW
    return ROSE


# ---------------------------------------------------------------------------
# Boot screen
# ---------------------------------------------------------------------------
BOOT_LINES = [
    ("INITIALIZING SUBSYSTEMS", None, None),
    ("MPU-6500 IMU", "OK", GREEN),
    ("PCA9685 SERVO DRIVER", "OK", GREEN),
    ("TB6612FNG MOTOR BRIDGE", "OK", GREEN),
    ("PID CONTROLLER", "READY", CYAN),
    ("BALANCE LOOP @ 100 Hz", "ACTIVE", YELLOW),
    ("SYSTEM ONLINE", None, CYAN),
]
BOOT_DURATION = 3.0


class BootScreen:
    def __init__(self, fonts):
        self.fonts = fonts
        self.t0 = None
        self.done = False

    def reset(self):
        self.t0 = None
        self.done = False

    def update(self, dt):
        if self.t0 is None:
            self.t0 = time.time()
        elapsed = time.time() - self.t0
        if elapsed >= BOOT_DURATION + 0.6:
            self.done = True

    def draw(self, surf):
        elapsed = time.time() - self.t0 if self.t0 else 0
        surf.fill(BG)

        fade_out = max(0.0, min(1.0, (elapsed - BOOT_DURATION) / 0.6))

        header_font = self.fonts["header"]
        mono = self.fonts["mono"]
        mono_bold = self.fonts["mono_bold"]

        # Header
        header_alpha = min(1.0, elapsed / 0.4)
        hdr = header_font.render("SBR · 01", True, WHITE)
        hdr.set_alpha(int(header_alpha * 255))
        hx = 40
        hy = 40
        surf.blit(hdr, (hx, hy))
        pygame.draw.line(surf, LINE2, (hx, hy + hdr.get_height() + 8),
                         (W - 40, hy + hdr.get_height() + 8))

        # Boot lines
        line_y = hy + hdr.get_height() + 28
        n = len(BOOT_LINES)
        per_line = (BOOT_DURATION - 0.5) / n

        for i, (label, status, color) in enumerate(BOOT_LINES):
            line_start = 0.3 + i * per_line
            line_elapsed = elapsed - line_start
            if line_elapsed < 0:
                break

            show_t = min(1.0, line_elapsed / 0.2)
            alpha = int(show_t * 255)
            x_off = int((1.0 - show_t) * -8)

            if i == n - 1:
                # SYSTEM ONLINE — bold, cyan
                txt = mono_bold.render("▸ " + label, True, color or CYAN)
                txt.set_alpha(alpha)
                surf.blit(txt, (hx + x_off, line_y))
            elif status is None:
                txt = mono.render("▸ " + label, True, DIM)
                txt.set_alpha(alpha)
                surf.blit(txt, (hx + x_off, line_y))
            else:
                lbl = mono.render("▸ " + label + " ", True, DIM)
                lbl.set_alpha(alpha)
                surf.blit(lbl, (hx + x_off, line_y))

                dot_w = W - 40 - hx - lbl.get_width() - mono_bold.size(status)[0] - 8
                dots_visible = min(1.0, line_elapsed / (per_line * 0.7))
                num_dots = max(0, int(dot_w / mono.size("·")[0]))
                dot_str = "·" * int(num_dots * dots_visible)
                ds = mono.render(dot_str, True, MUTE)
                ds.set_alpha(alpha)
                surf.blit(ds, (hx + x_off + lbl.get_width(), line_y))

                if dots_visible >= 1.0:
                    st = mono_bold.render(status, True, color)
                    st.set_alpha(alpha)
                    surf.blit(st, (W - 40 - st.get_width(), line_y))

            line_y += int(mono.get_linesize() * 1.6)

        # Progress bar
        bar_y = H - 32
        bar_h = 3
        bar_alpha = min(1.0, elapsed / 0.6)
        pct = min(1.0, elapsed / BOOT_DURATION)
        bar_rect = (hx, bar_y, W - 80, bar_h)
        if bar_alpha > 0:
            draw_gradient_bar(surf, bar_rect, CYAN, ORANGE, pct)

        # Fade out overlay
        if fade_out > 0:
            overlay = pygame.Surface((W, H))
            overlay.fill(BG)
            overlay.set_alpha(int(fade_out * 255))
            surf.blit(overlay, (0, 0))


# ---------------------------------------------------------------------------
# Face screen
# ---------------------------------------------------------------------------
class FaceScreen:
    def __init__(self, fonts):
        self.fonts = fonts
        self.t0 = time.time()
        self.wake_done = False
        self.wake_start = None
        self.blink_timer = 0.0
        self.blinking = False
        self.blink_phase = 0.0
        self.next_blink = 3.0

    def start_wake(self):
        self.wake_start = time.time()

    def update(self, dt, snap):
        if self.wake_start and not self.wake_done:
            if time.time() - self.wake_start > 0.6:
                self.wake_done = True

        if self.wake_done and not self.blinking:
            self.blink_timer += dt
            if self.blink_timer >= self.next_blink:
                self.blinking = True
                self.blink_phase = 0.0
                self.blink_timer = 0.0
                self.next_blink = 2.5 + 3.0 * abs(math.sin(time.time()))

        if self.blinking:
            self.blink_phase += dt
            if self.blink_phase > 0.3:
                self.blinking = False

    def draw(self, surf, snap):
        surf.fill(BG)
        state = snap["state"]
        pitch = snap["pitch"]
        connected = snap["connected"]

        cx, cy = W // 2, H // 2 - 15
        eye_gap = 72
        eye_w, eye_h = 44, 50
        eye_r = 14

        # Wake animation
        if self.wake_start and not self.wake_done:
            t = min(1.0, (time.time() - self.wake_start) / 0.5)
            eye_h_anim = int(4 + (eye_h - 4) * t)
        elif not self.wake_start:
            eye_h_anim = 4
        else:
            eye_h_anim = eye_h

        # Blink
        if self.blinking:
            p = self.blink_phase / 0.3
            if p < 0.5:
                squeeze = p * 2
            else:
                squeeze = (1.0 - p) * 2
            eye_h_anim = int(eye_h * (1.0 - squeeze * 0.85))

        # State modifications
        lid_top = 0
        lid_bot = 0
        pupil_ox = 0
        pupil_oy = 0
        eye_scale = 1.0
        expression_color = CYAN

        if state == "SIT":
            lid_top = int(eye_h_anim * 0.35)
            expression_color = DIM
        elif state == "STD":
            eye_scale = 1.15
            expression_color = YELLOW
        elif state == "BAL":
            lean = max(-1.0, min(1.0, pitch / 15.0))
            pupil_ox = int(lean * 14)
            pupil_oy = int(abs(lean) * 4)
            expression_color = CYAN

        # Fall
        if abs(pitch) > 30:
            eye_scale = 1.3
            eye_w_use = int(eye_h * eye_scale)
            eye_h_use = int(eye_h * eye_scale)
            eye_r_use = eye_w_use // 2
            expression_color = ROSE
        else:
            eye_w_use = int(eye_w * eye_scale)
            eye_h_use = int(eye_h_anim * eye_scale)
            eye_r_use = eye_r

        for side in (-1, 1):
            ex = cx + side * eye_gap
            ey = cy
            rect = pygame.Rect(0, 0, eye_w_use, eye_h_use)
            rect.center = (ex, ey)

            # Eye background (dark)
            pygame.draw.rect(surf, (20, 22, 30), rect, border_radius=eye_r_use)
            # Eye fill
            inner = rect.inflate(-4, -4)
            pygame.draw.rect(surf, expression_color, inner, border_radius=max(1, eye_r_use - 2))

            # Pupil
            pr = max(6, int(eye_w_use * 0.22))
            px = ex + pupil_ox + side * 2
            py = ey + pupil_oy
            pygame.draw.circle(surf, (20, 22, 30), (px, py), pr)
            # Highlight
            pygame.draw.circle(surf, WHITE, (px - pr // 3, py - pr // 3), max(2, pr // 3))

            # Lids
            if lid_top > 0:
                lid_rect = pygame.Rect(rect.left - 2, rect.top - 2, rect.width + 4, lid_top + 4)
                pygame.draw.rect(surf, BG, lid_rect)
            if lid_bot > 0:
                lid_rect = pygame.Rect(rect.left - 2, rect.bottom - lid_bot - 2, rect.width + 4, lid_bot + 4)
                pygame.draw.rect(surf, BG, lid_rect)

        # Status text
        mono = self.fonts["mono"]
        state_label = {"SIT": "SITTING", "STD": "STANDING UP", "BAL": "BALANCING"}.get(state, state)
        if abs(pitch) > 30:
            state_label = "FALL DETECTED"
        st = mono.render(state_label, True, expression_color)
        surf.blit(st, (cx - st.get_width() // 2, cy + int(eye_h * eye_scale) + 30))

        # Connection dot
        dot_c = GREEN if connected else ROSE
        pygame.draw.circle(surf, dot_c, (W - 20, 16), 5)

        # Hint
        hint = self.fonts["small"].render("tap → dashboard", True, MUTE)
        surf.blit(hint, (cx - hint.get_width() // 2, H - 22))


# ---------------------------------------------------------------------------
# Dashboard screen
# ---------------------------------------------------------------------------
class DashboardScreen:
    def __init__(self, fonts):
        self.fonts = fonts

    def draw(self, surf, snap):
        surf.fill(BG)
        mono = self.fonts["mono"]
        mono_bold = self.fonts["mono_bold"]
        small = self.fonts["small"]
        header = self.fonts["header"]
        big = self.fonts["big"]

        pitch = snap["pitch"]
        ml = snap["motor_l"]
        mr = snap["motor_r"]
        state = snap["state"]
        connected = snap["connected"]
        history = snap["history"]
        uptime = snap["uptime"]

        # Header bar
        pygame.draw.rect(surf, (14, 16, 22), (0, 0, W, 32))
        pygame.draw.line(surf, LINE2, (0, 32), (W, 32))

        hdr = mono_bold.render("SBR · 01", True, WHITE)
        surf.blit(hdr, (12, 7))

        dot_c = GREEN if connected else ROSE
        pygame.draw.circle(surf, dot_c, (hdr.get_width() + 24, 16), 5)
        link = small.render("LINKED" if connected else "NO DATA", True, dot_c)
        surf.blit(link, (hdr.get_width() + 34, 9))

        st_txt = mono_bold.render(state, True, CYAN)
        surf.blit(st_txt, (W - st_txt.get_width() - 12, 7))

        # Pitch number
        pc = pitch_color(pitch)
        sign = "+" if pitch >= 0 else ""
        ptxt = big.render(f"{sign}{pitch:.1f}°", True, pc)
        surf.blit(ptxt, (20, 44))

        plbl = small.render("PITCH", True, DIM)
        surf.blit(plbl, (20, 44 + ptxt.get_height() + 2))

        # Tilt indicator (circular)
        tilt_cx = 160
        tilt_cy = 80
        tilt_r = 32
        pygame.draw.circle(surf, LINE, (tilt_cx, tilt_cy), tilt_r, 1)
        pygame.draw.circle(surf, LINE, (tilt_cx, tilt_cy), tilt_r // 2, 1)
        # Horizon line
        pygame.draw.line(surf, MUTE, (tilt_cx - tilt_r, tilt_cy),
                         (tilt_cx + tilt_r, tilt_cy), 1)
        # Tilt dot
        angle_rad = math.radians(max(-45, min(45, pitch)))
        dx = int(math.sin(angle_rad) * (tilt_r - 6))
        dy = int(-math.cos(angle_rad) * (tilt_r - 6))
        pygame.draw.circle(surf, pc, (tilt_cx + dx, tilt_cy + dy), 5)
        pygame.draw.circle(surf, pc, (tilt_cx, tilt_cy), 3)

        # Pitch graph
        graph_x, graph_y = 210, 42
        graph_w, graph_h = W - graph_x - 14, 78
        pygame.draw.rect(surf, (14, 16, 22), (graph_x, graph_y, graph_w, graph_h))
        pygame.draw.rect(surf, LINE2, (graph_x, graph_y, graph_w, graph_h), 1)
        # Zero line
        mid_y = graph_y + graph_h // 2
        pygame.draw.line(surf, MUTE, (graph_x, mid_y), (graph_x + graph_w, mid_y), 1)

        if len(history) > 1:
            max_angle = 45.0
            pts = []
            for i, v in enumerate(history):
                px = graph_x + int(i * graph_w / HISTORY)
                py = mid_y - int((v / max_angle) * (graph_h // 2))
                py = max(graph_y + 1, min(graph_y + graph_h - 2, py))
                pts.append((px, py))
            if len(pts) >= 2:
                pygame.draw.lines(surf, CYAN, False, pts, 1)

        glbl = small.render("PITCH HISTORY", True, DIM)
        surf.blit(glbl, (graph_x + 4, graph_y + 2))

        # Motor gauges
        gauge_y = 134
        gauge_h = 18
        gauge_w = W - 110
        gauge_x = 94

        for i, (label, val) in enumerate(
            [("MOTOR L", ml), ("MOTOR R", mr)]
        ):
            gy = gauge_y + i * (gauge_h + 18)
            lbl = mono.render(label, True, DIM)
            surf.blit(lbl, (12, gy + 1))

            # Gauge background
            pygame.draw.rect(surf, (14, 16, 22), (gauge_x, gy, gauge_w, gauge_h))
            pygame.draw.rect(surf, LINE2, (gauge_x, gy, gauge_w, gauge_h), 1)

            # Center line
            center_x = gauge_x + gauge_w // 2
            pygame.draw.line(surf, MUTE, (center_x, gy), (center_x, gy + gauge_h), 1)

            # Fill
            norm = max(-255, min(255, val)) / 255.0
            fill_w = int(abs(norm) * (gauge_w // 2))
            fc = GREEN if abs(val) < 150 else (YELLOW if abs(val) < 220 else ORANGE)
            if norm >= 0:
                pygame.draw.rect(surf, fc, (center_x, gy + 1, fill_w, gauge_h - 2))
            else:
                pygame.draw.rect(surf, fc, (center_x - fill_w, gy + 1, fill_w, gauge_h - 2))

            # Value
            vtxt = mono_bold.render(f"{val:+d}", True, WHITE)
            surf.blit(vtxt, (gauge_x + gauge_w + 6, gy + 1))

        # Divider
        div_y = gauge_y + 2 * (gauge_h + 18) + 10
        pygame.draw.line(surf, LINE2, (12, div_y), (W - 12, div_y))

        # Bottom status row
        row_y = div_y + 8
        items = [
            ("STATE", state, CYAN),
            ("LOOP", "100 Hz", DIM),
            ("TELE", "20 Hz", DIM),
        ]
        ix = 12
        for label, val, vc in items:
            lt = small.render(label, True, MUTE)
            vt = mono.render(val, True, vc)
            surf.blit(lt, (ix, row_y))
            surf.blit(vt, (ix, row_y + lt.get_height() + 2))
            ix += max(lt.get_width(), vt.get_width()) + 28

        # Clock & uptime
        now = time.strftime("%H:%M:%S")
        ct = mono.render(now, True, DIM)
        surf.blit(ct, (W - ct.get_width() - 12, row_y))

        mins, secs = divmod(int(uptime), 60)
        hrs, mins = divmod(mins, 60)
        ut = small.render(f"UP {hrs:02d}:{mins:02d}:{secs:02d}", True, MUTE)
        surf.blit(ut, (W - ut.get_width() - 12, row_y + ct.get_height() + 2))

        # Hint
        hint = self.fonts["small"].render("tap → face", True, MUTE)
        surf.blit(hint, (W // 2 - hint.get_width() // 2, H - 18))


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------
def main():
    parser = argparse.ArgumentParser(description="SBR-01 Display")
    parser.add_argument("--demo", action="store_true", help="Run with fake telemetry")
    parser.add_argument("--windowed", action="store_true", help="Force windowed mode")
    parser.add_argument("--port", default=SERIAL_PORT, help="Serial port")
    args = parser.parse_args()

    os.environ.setdefault("SDL_FBDEV", "/dev/fb1")

    fullscreen = not args.windowed and "DISPLAY" not in os.environ

    pygame.init()

    flags = 0
    if fullscreen:
        flags = pygame.FULLSCREEN | pygame.NOFRAME
        pygame.mouse.set_visible(False)

    screen = pygame.display.set_mode((W, H), flags)
    pygame.display.set_caption("SBR-01")
    clock = pygame.time.Clock()

    # Fonts
    mono_name = "dejavusansmono"
    fallbacks = ["couriernew", "liberationmono", "mono"]
    chosen = mono_name
    if not pygame.font.match_font(mono_name):
        for fb in fallbacks:
            if pygame.font.match_font(fb):
                chosen = fb
                break
        else:
            chosen = None

    if chosen:
        mono_path = pygame.font.match_font(chosen)
        mono_bold_path = pygame.font.match_font(chosen, bold=True) or mono_path
    else:
        mono_path = None
        mono_bold_path = None

    def make_font(path, size):
        if path:
            return pygame.font.Font(path, size)
        return pygame.font.SysFont(None, size)

    fonts = {
        "mono": make_font(mono_path, 13),
        "mono_bold": make_font(mono_bold_path, 13),
        "header": make_font(mono_bold_path, 22),
        "big": make_font(mono_bold_path, 42),
        "small": make_font(mono_path, 11),
    }

    tele = Telemetry()

    if args.demo:
        t = threading.Thread(target=demo_generator, args=(tele,), daemon=True)
        t.start()
    else:
        t = threading.Thread(target=serial_reader, args=(tele, args.port, BAUD), daemon=True)
        t.start()

    boot = BootScreen(fonts)
    face = FaceScreen(fonts)
    dash = DashboardScreen(fonts)

    SCREEN_BOOT = 0
    SCREEN_FACE = 1
    SCREEN_DASH = 2
    current = SCREEN_BOOT

    running = True
    while running:
        dt = clock.tick(FPS) / 1000.0
        snap = tele.snapshot()

        for ev in pygame.event.get():
            if ev.type == pygame.QUIT:
                running = False
            elif ev.type == pygame.KEYDOWN:
                if ev.key in (pygame.K_ESCAPE, pygame.K_q):
                    running = False
                elif ev.key == pygame.K_SPACE:
                    if current == SCREEN_FACE:
                        current = SCREEN_DASH
                    elif current == SCREEN_DASH:
                        current = SCREEN_FACE
            elif ev.type == pygame.MOUSEBUTTONDOWN:
                if current == SCREEN_FACE:
                    current = SCREEN_DASH
                elif current == SCREEN_DASH:
                    current = SCREEN_FACE

        if current == SCREEN_BOOT:
            boot.update(dt)
            boot.draw(screen)
            if boot.done:
                current = SCREEN_FACE
                face.start_wake()
        elif current == SCREEN_FACE:
            face.update(dt, snap)
            face.draw(screen, snap)
        elif current == SCREEN_DASH:
            dash.draw(screen, snap)

        pygame.display.flip()

    pygame.quit()


if __name__ == "__main__":
    main()
