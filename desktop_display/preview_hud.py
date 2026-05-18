#!/usr/bin/env python3
"""
SOMA Desktop v6 — OO + LLM BAREMETAL
Spiral Galaxy BG | Giant Orbital Rings | Glassmorphism Panels
Esc=quit  F11=fullscreen  Type=chat  Enter=send

Voice Bridge: reads oo_voice_state.json (written by C bridge via UART)
to update vortex color, speed, waveform in real time.
"""
import sys, math, random, os, json, threading, time

try:
    import pygame
except ImportError:
    print("pip install pygame"); sys.exit(1)

# ─── Palette ───────────────────────────────────────────────────────────────────
BG          = (  2,   3,  18)
DEEPSPACE   = (  1,   2,  12)
CYAN        = (  0, 210, 255)
CYAN_MED    = (  0, 130, 170)
CYAN_DIM    = (  0,  55,  80)
CYAN_FAINT  = (  0,  18,  32)
WHITE       = (240, 248, 255)
WHITE_HOT   = (255, 255, 220)
GREEN       = (  0, 200,  70)
AMBER       = (255, 160,   0)
ORANGE_C    = (255, 100,  30)
RED         = (220,  40,  30)
RED_HOT     = (255,  80,  60)
YELLOW      = (220, 200,   0)
MAGENTA     = (160,   0, 200)
GOLD        = (255, 180,   0)
PURPLE      = (120,   0, 255)
GLASS_BG    = (  4,  12,  40)

STATE_COLORS = [CYAN, AMBER, RED, WHITE, (30, 80, 240)]
STATE_NAMES  = ["ACTIVE","DEGRADED","ISOLATED","EMERGENCY","SLEEPING"]
DPLUS_NAMES  = ["SOLAR","LUNAR","SAFE"]

def clamp(v, lo, hi): return max(lo, min(hi, v))
def lerp_c(a, b, t): return tuple(int(a[i]+(b[i]-a[i])*t) for i in range(3))
def dim_c(c, a): return tuple(clamp(int(v*a//255),0,255) for v in c[:3])

# ─── Voice Bridge (reads oo_voice_state.json written by UART bridge) ──────────
VOICE_STATE_FILE = os.path.join(os.path.dirname(__file__), "oo_voice_state.json")

# Emotion → vortex color table (mirrors oo_voice_desktop_bridge.h)
EMOTION_COLORS = {
    "FOCUSED":    (  0, 120, 255),  # blue
    "CURIOUS":    (  0, 210, 255),  # cyan
    "ALERT":      (255,  40,  30),  # red
    "DORMANT":    ( 20,  20,  80),  # dark indigo
    "PROUD":      (220, 180,   0),  # gold
    "CAUTIOUS":   (255, 140,   0),  # amber
    "SPEAKING":   (  0, 255, 180),  # teal (TTS active)
    "WAKE":       (255, 255, 220),  # white pulse on wake
}

class VoiceBridge:
    """Reads oo_voice_state.json in a background thread, updates shared state."""
    def __init__(self):
        self._lock = threading.Lock()
        self._state = {
            "emotion": "FOCUSED",
            "vortex_speed_mul": 1.0,
            "vortex_color": list(CYAN),
            "wake_pulse": 0,
            "glitch": 0,
            "waveform": [128]*64,
            "response": "",
            "inference": False,
        }
        self._running = True
        self._thread = threading.Thread(target=self._loop, daemon=True)
        self._thread.start()

    def _loop(self):
        while self._running:
            try:
                if os.path.exists(VOICE_STATE_FILE):
                    with open(VOICE_STATE_FILE, "r") as f:
                        raw = json.load(f)
                    emotion = raw.get("emotion", "FOCUSED")
                    col = EMOTION_COLORS.get(emotion, CYAN)
                    with self._lock:
                        self._state["emotion"]          = emotion
                        self._state["vortex_color"]     = list(col)
                        self._state["vortex_speed_mul"] = float(raw.get("vortex_speed_mul", 1.0))
                        self._state["wake_pulse"]       = int(raw.get("wake_pulse", 0))
                        self._state["glitch"]           = int(raw.get("glitch", 0))
                        self._state["inference"]        = bool(raw.get("inference", False))
                        wv = raw.get("waveform", [])
                        if len(wv) == 64:
                            self._state["waveform"] = wv
                        resp = raw.get("response", "")
                        if resp:
                            self._state["response"] = resp
            except Exception:
                pass
            time.sleep(0.05)  # 20 Hz poll

    def get(self):
        with self._lock:
            return dict(self._state)

    def stop(self):
        self._running = False

# ─── State ─────────────────────────────────────────────────────────────────────
class SomaState:
    def __init__(self):
        self.node_state=0; self.dplus_mode=0; self.warden_pressure=60
        self.swarm_count=3; self.uptime_sec=0; self.tokens_per_sec=50
        self.tokens_total=0; self.inference=True; self.glitch=0
        self.node_id="NODE_39"; self.model="LLM-40M"; self.input_buf=""
        self.events=[]; self.voice_wave=[128]*64; self.voice_active=False
        self.response_lines=[]
        self.arenas=[
            {"name":"WEIGHTS","used":3200,"total":4096},
            {"name":"KV-CACHE","used":800,"total":2048},
            {"name":"SCRATCH","used":200,"total":512},
            {"name":"ACTIVAT","used":400,"total":1024},
            {"name":"ZONE-C","used":80,"total":256},
        ]
        self.peers=[
            {"id":"NODE-001","state":0,"lat":10,"active":True},
            {"id":"NODE-002","state":1,"lat":25,"active":True},
            {"id":"NODE-003","state":0,"lat":40,"active":True},
            {"id":"NODE-004","state":4,"lat":15,"active":False},
        ]

    def update(self, tick):
        self.node_state=( tick//400)%5
        raw=math.sin(tick/30.0); self.warden_pressure=int((raw*0.5+0.5)*255)
        self.swarm_count=(tick//120)%5; self.uptime_sec=tick//60
        self.tokens_per_sec=int(45+math.sin(tick/20.0)*16)
        self.tokens_total+=max(0,self.tokens_per_sec)//60
        self.dplus_mode=(tick//500)%3; self.inference=(tick//80)%4!=0
        for i,a in enumerate(self.arenas):
            a["used"]=int(a["total"]*(0.30+(math.sin(tick/40.0+i)*0.5+0.5)*0.55))
        if tick%120==0:
            kinds=["INFERENCE","WARDEN","D+","ARENA","NODE"]
            descs=["STEP DONE","SPIKE","MODE SHIFT","REALLOC","JOINED"]
            self.events.append(f"{kinds[(tick//120)%5]}: {descs[(tick//120)%5]}")
            if len(self.events)>6: self.events.pop(0)
        for i,p in enumerate(self.peers):
            p["state"]=(i+tick//200)%5; p["lat"]=10+i*15+int(abs(math.sin(tick/25.0+i))*20)
            p["active"]=i<self.swarm_count
        for i in range(64):
            v=math.sin((tick*(2+i%4)/4.0+i*3)*math.pi/32)
            self.voice_wave[i]=int((v*0.5+0.5)*255)
        self.voice_active=(tick//150)%2==1
        if self.warden_pressure>220: self.glitch=8
        if self.glitch>0: self.glitch-=1

    def apply_voice_bridge(self, vb_state):
        """Override simulated fields with live voice bridge data (if any)."""
        if vb_state.get("wake_pulse", 0) > 0:
            self.glitch = max(self.glitch, 5)
        if vb_state.get("inference", False):
            self.inference = True
        if vb_state.get("glitch", 0) > 0:
            self.glitch = max(self.glitch, vb_state["glitch"])
        wv = vb_state.get("waveform")
        if wv and len(wv) == 64:
            self.voice_wave = list(wv)
            self.voice_active = True
        resp = vb_state.get("response", "")
        if resp and (not self.response_lines or self.response_lines[-1] != f"[OO] {resp}"):
            self.response_lines.append(f"[OO] {resp}")
            if len(self.response_lines) > 10:
                self.response_lines.pop(0)


# ─── Token particle ────────────────────────────────────────────────────────────
class TokenParticle:
    def __init__(self, x, y, tx, ty, col):
        dx,dy=tx-x,ty-y; d=math.hypot(dx,dy) or 1; spd=random.uniform(3,7)
        self.x,self.y=float(x),float(y); self.vx=dx/d*spd; self.vy=dy/d*spd
        self.col=col; self.life=1.0; self.decay=random.uniform(0.018,0.035)
        self.r=random.randint(1,3)
    def update(self):
        self.x+=self.vx; self.y+=self.vy; self.life-=self.decay
        self.vx*=0.97; self.vy*=0.97
    @property
    def dead(self): return self.life<=0


# ─── Main HUD ──────────────────────────────────────────────────────────────────
class SomaHUD:
    W0,H0=1280,720

    def __init__(self):
        pygame.init()
        self.screen=pygame.display.set_mode((self.W0,self.H0),pygame.RESIZABLE)
        pygame.display.set_caption("OO + LLM BAREMETAL — Operating Organism")
        pygame.mouse.set_visible(False)
        self.clock=pygame.time.Clock(); self.tick=0; self.state=SomaState()
        self.fullscreen=False; self.mx,self.my=self.W0//2,self.H0//2
        self.ticker_off=0; self.token_parts=[]; self.syn_lightning=[]
        self.hover_left=False; self.hover_right=False; self.hover_sphere=False

        mono="Courier New"
        self.f8 =pygame.font.SysFont(mono, 8,bold=True)
        self.f9 =pygame.font.SysFont(mono, 9,bold=True)
        self.f11=pygame.font.SysFont(mono,11,bold=True)
        self.f13=pygame.font.SysFont(mono,13,bold=True)
        self.f16=pygame.font.SysFont(mono,16,bold=True)
        self.f20=pygame.font.SysFont(mono,20,bold=True)
        self.f28=pygame.font.SysFont(mono,28,bold=True)

        random.seed(42)
        self._build_galaxy()
        self._build_synaptic()
        self._build_orbital_rings()

        # Voice Bridge — reads oo_voice_state.json (UART output from bare-metal)
        self._vbridge = VoiceBridge()
        self._vortex_color_override = None  # set by bridge when emotion changes

        # Matrix rain state
        cols_count = 80
        self.matrix_cols = [random.randint(0, 50) for _ in range(cols_count)]
        self.matrix_chars = [chr(c) for c in list(range(0x30, 0x3A)) + list(range(0x41, 0x5B)) +
                             [0x30A0 + i for i in range(96)]]
        self.matrix_col_speed = [random.uniform(0.3, 1.2) for _ in range(cols_count)]
        self.matrix_col_acc = [0.0] * cols_count

        # Energy wave rings state
        self.energy_waves = []  # list of (radius, max_r, alpha, col)
        self.next_wave_tick = 0

        # Typewriter boot state
        self.boot_lines = [
            (10,  "DIOP_MIND KERNEL v2.1.0", CYAN, 18),
            (18,  "INITIALIZING BARE-METAL SUBSTRATE...", CYAN_DIM, 11),
            (26,  "CPU: X86_64 ... OK", WHITE, 12),
            (34,  "GOP FRAMEBUFFER PROTOCOL ... ONLINE", GREEN, 12),
            (42,  "MOUNTING SYNAPTIC FABRIC ...", CYAN_DIM, 12),
            (50,  "FABRIC SYNCED. HANDSHAKE COMPLETE.", CYAN, 12),
            (56,  "LOADING WARDEN POLICY ... ACTIVE", AMBER, 12),
            (59,  "AWAKENING OPERATING ORGANISM...", MAGENTA, 14),
        ]
        self.typewriter_pos = {}  # line_tick -> revealed chars
        self.scan_y = 0          # CRT scan line y pos

        # Data stream particles
        self.data_streams = []  # list of {x, y, vx, vy, life, col, char}

        # Singularité (Singularity / Black Hole)
        self.singularity_angle = 0.0
        self.singularity_pulse = 0.0
        self.singularity_particles = [] # list of {ang, r, speed, col}
        for _ in range(100):
            self.singularity_particles.append({
                'ang': random.uniform(0, math.pi * 2),
                'r': random.uniform(80, 180),
                'spd': random.uniform(0.01, 0.04),
                'col': random.choice([GOLD, PURPLE, WHITE])
            })

        # Lissajous signature
        self.lissajous_trail = []  # list of (x, y, age)
        self.lissajous_t = 0.0

        # Stream de Conscience (token stream)
        self.token_stream_words = []
        self.token_stream_buf = []
        self.next_token_tick = 0
        self.SOMA_VOCAB = [
            "INFERENCE", "TENSOR", "LAYER", "ATTENTION", "GRADIENT",
            "WEIGHT", "NEURON", "EMBED", "QUERY", "KEY", "VALUE",
            "NORM", "SOFTMAX", "FORWARD", "SYNAPSE", "GATE", "CONTEXT",
            "TOKEN", "LOGIT", "DECODE", "ENCODE", "SOMA", "WARDEN",
            "PULSE", "SIGNAL", "CORTEX", "DREAM", "LATENT", "SPACE",
        ]

        # Ghost Panels (spectre mémoriel)
        self.ghost_positions = []  # ring buffer of (mx_norm, my_norm)
        self.ghost_max = 5

    def draw_heatmap(self, surf, x, y, w, h, pct, base_col):
        cols, rows = w // 5, h // 5
        for r in range(rows):
            for c in range(cols):
                block_pct = (c + r * cols) * 100 // (cols * rows)
                if block_pct < pct:
                    bright = math.sin(c*0.5 + r*0.7 - self.tick*0.2) * 0.5 + 0.5
                    bc = lerp_c(base_col, WHITE_HOT, bright * 0.5)
                    pygame.draw.rect(surf, bc, (x + c*5, y + r*5, 4, 4))
                else:
                    pygame.draw.rect(surf, (5, 10, 20), (x + c*5, y + r*5, 4, 4))

    def draw_radar(self, surf, cx, cy, r):
        pygame.draw.circle(surf, CYAN_DIM, (cx, cy), r, 1)
        pygame.draw.circle(surf, CYAN_DIM, (cx, cy), r//2, 1)
        pygame.draw.line(surf, CYAN_DIM, (cx - r, cy), (cx + r, cy), 1)
        pygame.draw.line(surf, CYAN_DIM, (cx, cy - r), (cx, cy + r), 1)
        
        angle = (self.tick * 0.05) % (2*math.pi)
        sx = cx + int(r * math.cos(angle))
        sy = cy + int(r * math.sin(angle))
        pygame.draw.line(surf, GREEN, (cx, cy), (sx, sy), 2)
        
        for i in range(1, 8):
            trail_ang = angle - i * 0.05
            tx = cx + int(r * math.cos(trail_ang))
            ty = cy + int(r * math.sin(trail_ang))
            fade = max(10, 255 - i * 30)
            pygame.draw.line(surf, (0, fade, fade//3), (cx, cy), (tx, ty), 2)
            
        for b in range(4):
            b_ang = (b * 2.1 + 0.5) % (2*math.pi)
            b_dist = r//4 + (b*12) % (r - r//4)
            bx = cx + int(b_dist * math.cos(b_ang))
            by = cy + int(b_dist * math.sin(b_ang))
            
            diff = (angle - b_ang) % (2*math.pi)
            if diff < 0.5 or diff > 2*math.pi - 0.5:
                bc = WHITE
            else:
                bc = CYAN_DIM
            pygame.draw.rect(surf, bc, (bx-1, by-1, 3, 3))
            pygame.draw.rect(surf, bc, (bx-1, by-1, 3, 3))

    # ── Aurora Borealis ────────────────────────────────────────────────────────
    def render_aurora(self, surf):
        sw, sh = self.sw(), self.sh()
        t = self.tick * 0.008
        au = pygame.Surface((sw, sh//2), pygame.SRCALPHA)
        bands = [
            (0.18, (0, 180,  80), 0.0),
            (0.30, (0, 100, 200), 0.7),
            (0.22, (80,  0, 200), 1.4),
            (0.14, (0, 210, 160), 2.1),
        ]
        for amp, col, phase in bands:
            pts = []
            for x in range(0, sw + 4, 4):
                freq1 = math.sin(x * 0.006 + t + phase)
                freq2 = math.sin(x * 0.014 - t * 1.3 + phase * 1.7)
                y = int(sh * 0.18 + sh * amp * (freq1 * 0.6 + freq2 * 0.4))
                pts.append((x, max(0, min(sh//2 - 1, y))))
            for i in range(len(pts)-1):
                dx = abs(pts[i+1][1] - pts[i][1])
                a = max(0, 60 - dx * 4)
                pygame.draw.line(au, (*col, a), pts[i], pts[i+1], 2)
                # Fade downward
                for dy in range(1, 30, 3):
                    ya, yb = pts[i][1]+dy, pts[i+1][1]+dy
                    if ya < sh//2 and yb < sh//2:
                        fade_a = max(0, int(a * (1 - dy/30.0)))
                        pygame.draw.line(au, (*col, fade_a),
                                        (pts[i][0], ya), (pts[i+1][0], yb), 1)
        surf.blit(au, (0, 0))

    # ── Cymatics Chladni Pattern ───────────────────────────────────────────────
    def render_cymatics(self, surf, cx, cy, r, st):
        n = 1 + (st.warden_pressure // 50) % 5  # nodal count changes with warden
        m = 1 + (st.tokens_per_sec  // 15) % 5
        t = self.tick * 0.018
        size = r * 2
        cy_surf = pygame.Surface((size, size), pygame.SRCALPHA)
        step = 4
        for xi in range(0, size, step):
            for yi in range(0, size, step):
                # Chladni: abs(sin(n*pi*x)*sin(m*pi*y)) near zero → nodal line
                xn = (xi / size - 0.5) * 2.0
                yn = (yi / size - 0.5) * 2.0
                dist = math.hypot(xn, yn)
                if dist > 1.0: continue
                val = abs(math.sin(n * math.pi * xn + t) *
                          math.sin(m * math.pi * yn + t * 0.7))
                if val < 0.18:
                    brightness = int((1 - val / 0.18) * 200)
                    col = lerp_c(CYAN_DIM, RED if st.warden_pressure > 180
                                 else AMBER if st.warden_pressure > 120
                                 else CYAN, val / 0.18)
                    pygame.draw.rect(cy_surf, (*col, brightness),
                                     (xi, yi, step-1, step-1))
        surf.blit(cy_surf, (cx - r, cy - r))
        self.tc(surf, cx, cy + r + 10,
                f"CYMATICS  n={n} m={m}", CYAN_DIM, 9)

    # ── DNA Double Helix ───────────────────────────────────────────────────────
    def render_dna_helix(self, surf, cx, y0, h, st):
        scroll = self.tick * 0.06
        n_rungs = 20
        strand_a = []
        strand_b = []
        for i in range(n_rungs + 1):
            frac = i / n_rungs
            y = int(y0 + frac * h)
            ang = frac * 4 * math.pi + scroll
            amp = int(self.W(50) * (0.5 + 0.5 * math.sin(self.tick * 0.01)))
            ax = cx + int(amp * math.cos(ang))
            bx = cx + int(amp * math.cos(ang + math.pi))
            depth_a = (math.cos(ang) + 1) * 0.5
            depth_b = (math.cos(ang + math.pi) + 1) * 0.5
            strand_a.append((ax, y, depth_a))
            strand_b.append((bx, y, depth_b))

        for i in range(len(strand_a) - 1):
            # Strand A (CYAN)
            ax1, ay1, da1 = strand_a[i];   ax2, ay2, da2 = strand_a[i+1]
            col_a = lerp_c(CYAN_DIM, CYAN, (da1+da2)*0.5)
            pygame.draw.line(surf, col_a, (ax1, ay1), (ax2, ay2), 2)
            # Strand B (MAGENTA)
            bx1, by1, db1 = strand_b[i];   bx2, by2, db2 = strand_b[i+1]
            col_b = lerp_c(dim_c(MAGENTA, 80), MAGENTA, (db1+db2)*0.5)
            pygame.draw.line(surf, col_b, (bx1, by1), (bx2, by2), 2)

        # Rungs (horizontal bridges)
        for i in range(0, n_rungs, 2):
            ax, ay, da = strand_a[i];  bx, by, db = strand_b[i]
            mid_depth = (da + db) * 0.5
            if mid_depth > 0.35:
                rc = lerp_c(CYAN_DIM, WHITE_HOT, mid_depth - 0.35)
                pygame.draw.line(surf, rc, (ax, ay), (bx, by), 1)
                # Bright node at each end
                pygame.draw.circle(surf, rc, (ax, ay), 2)
                pygame.draw.circle(surf, rc, (bx, by), 2)

        self.tc(surf, cx, y0 - 10, "DNA MODEL SIGNATURE", dim_c(CYAN, 160), 9)

    # ── Vector Scope (XY Oscilloscope) ────────────────────────────────────────
    def render_vector_scope(self, surf, cx, cy, r, st):
        """Classic XY phosphor oscilloscope — X=voice_wave, Y=phase-shifted."""
        wave = st.voice_wave  # 64 samples 0-255
        n = len(wave)
        pts = []
        for i in range(n):
            x_val = (wave[i] / 255.0) * 2.0 - 1.0
            y_val = (wave[(i + n//4) % n] / 255.0) * 2.0 - 1.0
            px = cx + int(x_val * (r - 4))
            py = cy + int(y_val * (r - 4))
            pts.append((px, py))

        # Dark glass BG
        vs_bg = pygame.Surface((r*2+4, r*2+4), pygame.SRCALPHA)
        pygame.draw.circle(vs_bg, (0, 20, 10, 180), (r+2, r+2), r)
        pygame.draw.circle(vs_bg, (0, 60, 30, 60),  (r+2, r+2), r, 1)
        surf.blit(vs_bg, (cx-r-2, cy-r-2))

        # Crosshairs
        pygame.draw.line(surf, (0, 40, 20), (cx-r, cy), (cx+r, cy), 1)
        pygame.draw.line(surf, (0, 40, 20), (cx, cy-r), (cx, cy+r), 1)

        # Trace with phosphor glow
        sc_surf = pygame.Surface((r*2+4, r*2+4), pygame.SRCALPHA)
        wp = st.warden_pressure / 255.0
        trace_col = lerp_c(GREEN, RED, wp)
        for i in range(len(pts)-1):
            pygame.draw.line(sc_surf, (*trace_col, 180), pts[i], pts[i+1], 1)
            # Glow dot on each sample
            if i % 4 == 0:
                pygame.draw.circle(sc_surf, (*WHITE_HOT, 60), pts[i], 2)
        surf.blit(sc_surf, (cx-r-2, cy-r-2))

        pygame.draw.circle(surf, GREEN, pts[-1], 2)
        self.tc(surf, cx, cy + r + 10, "VECTOR SCOPE", dim_c(GREEN, 180), 9)

    def render_matrix_rain(self, surf, alpha=180):
        sw, sh = self.sw(), self.sh()
        col_w = sw // len(self.matrix_cols)
        mat_surf = pygame.Surface((sw, sh), pygame.SRCALPHA)
        f = self.f11
        for i, drop_y in enumerate(self.matrix_cols):
            x = i * col_w
            for j in range(min(20, drop_y + 1)):
                y = (drop_y - j) * 14 % sh
                intensity = max(0.0, 1.0 - j / 18.0)
                if j == 0:
                    r2 = f.render(random.choice(self.matrix_chars), True, WHITE)
                    r2.set_alpha(alpha)
                else:
                    g_val = int(intensity * 200)
                    r2 = f.render(random.choice(self.matrix_chars), True, (0, g_val, int(g_val * 0.4)))
                    r2.set_alpha(int(alpha * intensity))
                mat_surf.blit(r2, (x, y))
        surf.blit(mat_surf, (0, 0))
        for i in range(len(self.matrix_cols)):
            self.matrix_col_acc[i] += self.matrix_col_speed[i]
            if self.matrix_col_acc[i] >= 1.0:
                self.matrix_cols[i] = (self.matrix_cols[i] + 1) % 60
                self.matrix_col_acc[i] -= 1.0

    # ── CRT Scanline Overlay ───────────────────────────────────────────────────
    def render_scanlines(self, surf):
        sw, sh = self.sw(), self.sh()
        scan_surf = pygame.Surface((sw, sh), pygame.SRCALPHA)
        for y in range(0, sh, 3):
            pygame.draw.line(scan_surf, (0, 0, 0, 55), (0, y), (sw, y), 1)
        self.scan_y = (self.scan_y + 2) % sh
        for dy in range(4):
            a = int(30 * (1 - dy / 4.0))
            y2 = (self.scan_y + dy) % sh
            pygame.draw.line(scan_surf, (0, 230, 255, a), (0, y2), (sw, y2), 1)
        surf.blit(scan_surf, (0, 0))

    # ── Energy Pulse Waves ─────────────────────────────────────────────────────
    def render_energy_waves(self, surf, cx, cy, st):
        if self.tick >= self.next_wave_tick:
            wp = st.warden_pressure
            col = RED if wp > 180 else AMBER if wp > 120 else CYAN
            self.energy_waves.append([0, self.H(220), 200, col])
            self.next_wave_tick = self.tick + random.randint(30, 60)
        wave_surf = pygame.Surface((self.sw(), self.sh()), pygame.SRCALPHA)
        for w in self.energy_waves[:]:
            r, max_r, alpha, col = w
            if alpha > 5 and r < max_r:
                t = r / max_r
                a = int(alpha * (1 - t) * (1 - t))
                width = max(1, int(3 * (1 - t)))
                pygame.draw.circle(wave_surf, (*col, a), (cx, cy), r, width)
                w[0] += 4
                w[2] = max(0, alpha - 3)
            else:
                self.energy_waves.remove(w)
        surf.blit(wave_surf, (0, 0))

    # ── Data Streams (inter-panel particle flow) ───────────────────────────────
    def render_data_streams(self, surf):
        sw, sh = self.sw(), self.sh()
        if self.tick % 6 == 0:
            self.data_streams.append({
                'x': float(self.W(195)), 'y': float(random.randint(self.H(50), self.H(570))),
                'vx': random.uniform(1.5, 3.0), 'vy': random.uniform(-0.3, 0.3),
                'life': 1.0, 'col': CYAN, 'char': random.choice('01ABCDEF')
            })
        if self.tick % 7 == 0:
            self.data_streams.append({
                'x': float(sw - self.W(202) - 14), 'y': float(random.randint(self.H(50), self.H(570))),
                'vx': random.uniform(-3.0, -1.5), 'vy': random.uniform(-0.3, 0.3),
                'life': 1.0, 'col': MAGENTA, 'char': random.choice('01ABCDEF')
            })
        ds_surf = pygame.Surface((sw, sh), pygame.SRCALPHA)
        f = self.f9
        for p in self.data_streams[:]:
            p['x'] += p['vx']; p['y'] += p['vy']; p['life'] -= 0.025
            if p['life'] <= 0:
                self.data_streams.remove(p)
                continue
            try:
                r2 = f.render(p['char'], True, p['col'])
                r2.set_alpha(int(p['life'] * 180))
                ds_surf.blit(r2, (int(p['x']), int(p['y'])))
            except Exception:
                pass
        surf.blit(ds_surf, (0, 0))

    # ── Chromatic Aberration Title ─────────────────────────────────────────────
    def draw_title_glitch(self, surf, cx, y, text, col, size, intensity=2):
        f = self.f(size)
        r2 = f.render(text, True, (255, 0, 0))
        surf.blit(r2, (cx - r2.get_width()//2 + intensity, y - 1))
        r3 = f.render(text, True, (0, 0, 255))
        surf.blit(r3, (cx - r3.get_width()//2 - intensity, y + 1))
        r4 = f.render(text, True, col)
        surf.blit(r4, (cx - r4.get_width()//2, y))

    # ── L'Œil de SOMA ─────────────────────────────────────────────────────────
    # ── Singularité : Horizon d'Événements ──────────────────────────────────────
    def render_singularity(self, surf, cx, cy, st):
        """Une singularité gravitationnelle avec disque d'accrétion doré et violet."""
        wp = st.warden_pressure / 255.0
        inf = st.inference
        
        self.singularity_angle += 0.02 + wp * 0.05
        self.singularity_pulse = math.sin(self.tick * 0.05) * 0.5 + 0.5
        
        base_r = self.H(65)
        core_r = base_r + int(self.singularity_pulse * 5)
        
        # 1. Glow extérieur (Violet/Bleu)
        glow_surf = pygame.Surface((self.sw(), self.sh()), pygame.SRCALPHA)
        for r_off in range(40, 0, -5):
            alpha = int(20 * (1.0 - r_off/40.0))
            pygame.draw.circle(glow_surf, (*PURPLE, alpha), (cx, cy), core_r + r_off)
        surf.blit(glow_surf, (0, 0))

        # 2. Disque d'accrétion (Anneaux inclinés)
        # On dessine plusieurs ellipses pour simuler le disque
        for i in range(12):
            ang_off = i * 0.1
            sw_val = int(base_r * 2.8 + math.sin(self.singularity_angle + ang_off) * 10)
            sh_val = int(base_r * 0.6 + math.cos(self.singularity_angle * 0.5) * 5)
            
            # Rotation de l'ellipse par rapport à l'inclinaison
            # En pygame on simule ça par des points ou une surface tournée
            # Ici on reste simple avec des ellipses horizontales/inclinées
            rect = pygame.Rect(0, 0, sw_val, sh_val)
            rect.center = (cx, cy)
            
            col = GOLD if i % 2 == 0 else PURPLE
            alpha = int(100 * (1.0 - i/12.0))
            
            # On dessine juste le bord
            temp_s = pygame.Surface((sw_val+4, sh_val+4), pygame.SRCALPHA)
            pygame.draw.ellipse(temp_s, (*col, alpha), (2, 2, sw_val, sh_val), 2)
            # On pourrait pivoter temp_s mais restons sur l'esthétique du screenshot
            surf.blit(temp_s, (rect.x, rect.y))

        # 3. Particules orbitales
        for p in self.singularity_particles:
            p['ang'] += p['spd'] * (1.0 + wp * 2.0)
            # Rayon dynamique avec oscillation
            r = p['r'] + math.sin(self.tick * 0.02 + p['ang']) * 10
            px = cx + int(r * math.cos(p['ang']))
            py = cy + int(r * 0.3 * math.sin(p['ang'])) # Disque plat
            
            alpha = int(180 * (0.5 + 0.5 * math.cos(p['ang']))) # Fade derrière
            size = 1 if alpha < 100 else 2
            pygame.draw.circle(surf, (*p['col'], alpha), (px, py), size)

        # 4. Le Cœur : Neural Plasma Core
        # Au lieu d'un simple cercle, on crée un effet de plasma volumétrique
        
        # Halo de base (Bleu/Cyan profond)
        for r_glow in range(15, 0, -3):
            ga = int(80 * (r_glow/15.0))
            pygame.draw.circle(surf, (*CYAN, ga), (cx, cy), core_r + r_glow)

        # Sphère de Plasma (Gradient radial simulé)
        for r_step in range(core_r, 0, -4):
            frac = r_step / core_r
            # De blanc pur au centre à Cyan/Bleu sur les bords
            col = lerp_c(WHITE_HOT, CYAN, 1.0 - frac)
            # Ajout d'une instabilité sur les bords
            off_x = int(math.sin(self.tick * 0.1 + r_step) * 2)
            off_y = int(math.cos(self.tick * 0.12 + r_step) * 2)
            pygame.draw.circle(surf, col, (cx + off_x, cy + off_y), r_step)

        # 5. Boucles de Plasma (Prominences / Magnetic Loops)
        # Ces boucles "sortent" du cœur et y reviennent
        n_loops = 6 if inf else 3
        for i in range(n_loops):
            l_ang = self.singularity_angle * 0.5 + i * (math.pi * 2 / n_loops)
            l_ext = core_r + 10 + int(math.sin(self.tick * 0.05 + i) * 15)
            # On dessine une petite courbe (arc)
            lx = cx + int(l_ext * math.cos(l_ang))
            ly = cy + int(l_ext * 0.4 * math.sin(l_ang))
            
            l_col = WHITE if inf else CYAN_DIM
            pygame.draw.circle(surf, l_col, (lx, ly), 2)
            # Traînée de la boucle
            pygame.draw.line(surf, (*l_col, 100), (cx, cy), (lx, ly), 1)

        # 6. Horizon des Événements (Anneau de distorsion)
        # Un anneau blanc très fin et brillant avec un "glitch" de scanline
        pygame.draw.circle(surf, WHITE, (cx, cy), core_r + 2, 1)
        if self.tick % 4 == 0:
            pygame.draw.line(surf, WHITE, (cx - core_r - 10, cy), (cx + core_r + 10, cy), 1)

        # 7. Singularity Core (Le point de densité infinie)
        # Un tout petit point noir au centre exact avec un halo de "lens flare"
        pygame.draw.circle(surf, (0, 0, 0), (cx, cy), 4)
        for fr in range(30, 0, -5):
            fa = int(100 * (1.0 - fr/30.0))
            pygame.draw.circle(surf, (*WHITE, fa), (cx, cy), fr, 1)

        # 8. Titres et Labels
        self.draw_title_glitch(surf, cx, cy - core_r - self.H(70), "HORIZON D'ÉVÉNEMENTS", GOLD, 20, intensity=2)
        self.tc(surf, cx, cy - core_r - self.H(45), "SINGULARITÉ NEURALE ACTIVE", WHITE, 9)
        
        # Flux de données orbitaux (Labels tournants)
        for i, label in enumerate(["SOMA_CORE", "INFERENCE_VORTEX", "WARDEN_GATE"]):
            la = self.singularity_angle * 0.3 + i * (math.pi * 2 / 3)
            lx = cx + int((core_r + 40) * math.cos(la))
            ly = cy + int((core_r + 40) * 0.5 * math.sin(la))
            self.tc(surf, lx, ly, label, PURPLE, 8)

    # ── Signature de Lissajous ─────────────────────────────────────────────────
    def render_lissajous(self, surf, cx, cy, w, h, st):
        """Biométrie mathématique: figure de Lissajous unique par état système."""
        tps = st.tokens_per_sec
        wp  = st.warden_pressure

        # Map metrics to Lissajous parameters
        a = 1 + (tps // 20) % 4       # x frequency: 1-4
        b = 1 + (wp  // 60) % 4       # y frequency: 1-4
        delta = (wp / 255.0) * math.pi  # phase delta

        # Advance parameter
        self.lissajous_t += 0.04
        t = self.lissajous_t

        # Compute new point
        px = cx + int((w // 2 - 8) * math.sin(a * t + delta))
        py = cy + int((h // 2 - 8) * math.sin(b * t))
        self.lissajous_trail.append((px, py, 0))

        # Age trail, keep last 300 points
        self.lissajous_trail = [(x, y, age + 1) for (x, y, age) in self.lissajous_trail]
        if len(self.lissajous_trail) > 300:
            self.lissajous_trail = self.lissajous_trail[-300:]

        # Draw trail with fading
        lis_surf = pygame.Surface((self.sw(), self.sh()), pygame.SRCALPHA)
        n = len(self.lissajous_trail)
        for i, (lx, ly, age) in enumerate(self.lissajous_trail):
            t_norm = i / max(1, n - 1)
            alpha = int(t_norm * 200)
            col = lerp_c(MAGENTA, CYAN, t_norm)
            pygame.draw.circle(lis_surf, (*col, alpha), (lx, ly), 1)
        surf.blit(lis_surf, (0, 0))

        # Bright head
        if self.lissajous_trail:
            hx, hy, _ = self.lissajous_trail[-1]
            pygame.draw.circle(surf, WHITE_HOT, (hx, hy), 3)

        # Label
        label_col = MAGENTA if a == b else CYAN
        self.tc(surf, cx, cy - h//2 - 10, f"Ψ BIOMETRIC  a={a} b={b}", label_col, 9)

    # ── Stream de Conscience ───────────────────────────────────────────────────
    def render_token_stream(self, surf, x, y, w, h, st):
        """Flux de tokens avec phosphor glow — les pensées de l'OO en direct."""
        if self.tick >= self.next_token_tick and st.inference:
            word = random.choice(self.SOMA_VOCAB)
            col = random.choice([CYAN, WHITE, GREEN, AMBER, MAGENTA])
            glow = random.random() > 0.7  # 30% bright flash
            self.token_stream_buf.append({'word': word, 'col': col, 'glow': glow,
                                         'born': self.tick, 'life': 1.0})
            self.next_token_tick = self.tick + random.randint(8, 25)

        # Age tokens
        line_h = 16
        max_lines = h // line_h
        if len(self.token_stream_buf) > max_lines * 6:
            self.token_stream_buf = self.token_stream_buf[-max_lines * 4:]

        # Build display lines (word-wrap style)
        ts_surf = pygame.Surface((w, h), pygame.SRCALPHA)
        cursor_x, cursor_y = 0, 0
        for tok in self.token_stream_buf:
            age = self.tick - tok['born']
            tok['life'] = max(0.0, 1.0 - age / 240.0)
            if tok['life'] <= 0:
                continue
            word_w = self.f9.size(tok['word'] + ' ')[0]
            if cursor_x + word_w > w:
                cursor_x = 0; cursor_y += line_h
            if cursor_y + line_h > h:
                break

            alpha = int(tok['life'] * 220)
            r2 = self.f9.render(tok['word'], True, tok['col'])
            r2.set_alpha(alpha)
            ts_surf.blit(r2, (cursor_x, cursor_y))

            # Phosphor glow on fresh tokens
            if tok['glow'] and age < 12:
                glow_alpha = int((1 - age / 12.0) * 140)
                glow_r = self.f11.render(tok['word'], True, WHITE_HOT)
                glow_r.set_alpha(glow_alpha)
                ts_surf.blit(glow_r, (cursor_x - 1, cursor_y - 1))

            cursor_x += word_w

        surf.blit(ts_surf, (x, y))
        self.tc(surf, x + w//2, y - 11, "◈ STREAM OF CONSCIOUSNESS ◈", CYAN_DIM, 9)

    # ── Spectre Mémoriel (Ghost Panels) ───────────────────────────────────────
    def render_ghost_panels(self, surf, mx_norm, my_norm):
        """Laisse des traces fantômes des panneaux pour créer une profondeur temporelle."""
        # Record current position
        self.ghost_positions.append((mx_norm, my_norm))
        if len(self.ghost_positions) > self.ghost_max * 4:
            self.ghost_positions = self.ghost_positions[-self.ghost_max * 4:]

        sw, sh = self.sw(), self.sh()
        # Draw ghosts from oldest to newest
        step = max(1, len(self.ghost_positions) // self.ghost_max)
        ghost_surf = pygame.Surface((sw, sh), pygame.SRCALPHA)
        for gi, idx in enumerate(range(0, len(self.ghost_positions) - step, step)):
            gx_norm, gy_norm = self.ghost_positions[idx]
            alpha = int(6 + gi * 4)  # oldest=darkest

            # Left panel ghost
            pw_l = self.W(195); ph_p = self.H(530)
            gpx_l = 14 + int(gx_norm * 30)
            gpy_l = 44 + int(gy_norm * 30)
            pygame.draw.rect(ghost_surf, (*CYAN_DIM, alpha),
                             (gpx_l, gpy_l, pw_l, ph_p), 1)

            # Right panel ghost
            pw_r = self.W(202)
            gpx_r = sw - pw_r - 14 + int(gx_norm * 30)
            gpy_r = 44 + int(gy_norm * 30)
            pygame.draw.rect(ghost_surf, (*MAGENTA, alpha),
                             (gpx_r, gpy_r, pw_r, ph_p), 1)

        surf.blit(ghost_surf, (0, 0))

    # ── Build galaxy texture (static, pre-rendered) ───────────────────────
    def _build_galaxy(self):
        W,H=self.W0,self.H0
        self.galaxy_surf=pygame.Surface((W,H))
        self.galaxy_surf.fill(BG)

        for y in range(H):
            frac=y/H
            r=int(2+frac*3); g=int(3+frac*4); b=int(18+frac*14)
            pygame.draw.line(self.galaxy_surf,(r,g,b),(0,y),(W,y))

        rng=random.Random(1)
        for _ in range(1800):
            x=rng.randint(0,W-1); y=rng.randint(0,H-1)
            br=rng.randint(15,70)
            self.galaxy_surf.set_at((x,y),(br,br,int(br*1.3)))

        blobs=[
            (W*0.25,H*0.30, 340,170, 12, 40,140, 30),
            (W*0.70,H*0.18, 280,140, 25, 18,120, 26),
            (W*0.50,H*0.65, 400,200,  6, 32,100, 24),
            (W*0.85,H*0.72, 250,125, 40, 24, 80, 22),
            (W*0.12,H*0.78, 300,160, 14, 44,110, 20),
            (W*0.62,H*0.10, 320,140, 10, 24,140, 28),
            (W*0.38,H*0.50, 260,135, 30, 16, 88, 18),
            (W*0.78,H*0.42, 200,110, 80, 58, 20, 22),
            (W*0.20,H*0.55, 230,120, 60, 48, 18, 18),
        ]
        tmp_neb=pygame.Surface((W,H),pygame.SRCALPHA)
        for bx,by,bw,bh,r,g,b,a in blobs:
            for s in range(16,0,-1):
                frac=s/16.0; ea=int(a*(1-frac)*0.6)
                if ea<=0: continue
                ew,eh=int(bw*frac),int(bh*frac)
                t2=pygame.Surface((ew*2+4,eh*2+4),pygame.SRCALPHA)
                t2.fill((0,0,0,0))
                pygame.draw.ellipse(t2,(r,g,b,ea),(2,2,ew*2,eh*2))
                tmp_neb.blit(t2,(int(bx)-ew-2,int(by)-eh-2),
                             special_flags=pygame.BLEND_RGBA_ADD)
        self.galaxy_surf.blit(tmp_neb,(0,0))

        cx_g,cy_g=W*0.50,H*0.47
        n_arms=4
        arm_rng=random.Random(77)
        for arm in range(n_arms):
            arm_off=arm*2*math.pi/n_arms
            for i in range(700):
                t2=i/700.0
                radius=(t2**0.6)*max(W,H)*0.48
                angle=arm_off+t2*3.8*math.pi+arm_rng.uniform(-0.18,0.18)
                sx=cx_g+math.cos(angle)*radius+arm_rng.gauss(0,radius*0.08)
                sy=cy_g+math.sin(angle)*radius*0.48+arm_rng.gauss(0,radius*0.06)
                if not(0<=sx<W and 0<=sy<H): continue
                br_base=int((1-t2)*220+20)
                br=arm_rng.randint(max(10,br_base-60),min(255,br_base+30))
                if t2<0.25:
                    c=(min(255,br+30),min(255,br+20),br)
                elif t2<0.55:
                    c=(br,br,min(255,br+40))
                else:
                    c=(int(br*0.3),int(br*0.7),min(255,br+60))
                if arm_rng.random()<0.04:
                    for _ in range(8):
                        ox,oy=arm_rng.randint(-3,3),arm_rng.randint(-2,2)
                        nx2,ny2=int(sx+ox),int(sy+oy)
                        if 0<=nx2<W and 0<=ny2<H:
                            bc=min(255,br+50)
                            self.galaxy_surf.set_at((nx2,ny2),(bc,bc,min(255,bc+20)))
                else:
                    self.galaxy_surf.set_at((int(sx),int(sy)),c)

        core_surf=pygame.Surface((W,H),pygame.SRCALPHA)
        for cr,ca in [(200,4),(130,8),(80,14),(50,22),(28,35),(14,55),(6,100)]:
            pygame.draw.circle(core_surf,(200,220,255,ca),(int(cx_g),int(cy_g)),cr)
        for cr2,ca2 in [(5,180),(2,255)]:
            pygame.draw.circle(core_surf,(255,255,240,ca2),(int(cx_g),int(cy_g)),cr2)
        self.galaxy_surf.blit(core_surf,(0,0))

        rng2=random.Random(33)
        for _ in range(80):
            sx,sy=rng2.randint(0,W-1),rng2.randint(0,H-1)
            br=rng2.randint(160,255); sz=rng2.randint(1,3)
            self.galaxy_surf.set_at((sx,sy),(br,br,min(255,br+20)))
            if sz>=2:
                for d in range(1,sz+4):
                    a2=max(0,br-d*45)
                    for ox2,oy2 in [(d,0),(-d,0),(0,d),(0,-d)]:
                        nx2,ny2=sx+ox2,sy+oy2
                        if 0<=nx2<W and 0<=ny2<H:
                            self.galaxy_surf.set_at((nx2,ny2),(a2,a2,min(255,a2+15)))

        rng3=random.Random(9)
        self.star_layers=[]
        for count,bmin,bmax,spd in [(60,30,80,0.003),(80,60,140,0.007),(35,130,210,0.014)]:
            stars=[(rng3.randint(0,W-1),rng3.randint(0,H-1),
                    rng3.randint(bmin,bmax),spd,rng3.uniform(0,math.pi*2))
                   for _ in range(count)]
            self.star_layers.append(stars)

    def _build_synaptic(self):
        random.seed(11)
        self.syn_nodes=[(random.uniform(0,1),random.uniform(0,1),random.uniform(0,1))
                        for _ in range(55)]
        self.syn_edges=[]
        for i in range(55):
            for j in range(i+1,55):
                dx=self.syn_nodes[i][0]-self.syn_nodes[j][0]
                dy=self.syn_nodes[i][1]-self.syn_nodes[j][1]
                if dx*dx+dy*dy<0.065: self.syn_edges.append((i,j))

    def _build_orbital_rings(self):
        self.rings=[
            (0.22, 0.06, 0.18, 0.003, CYAN,       1, 120),
            (0.32, 0.09, 0.28, 0.002, CYAN,       1,  90),
            (0.42, 0.13, 0.38,-0.0015,lerp_c(CYAN,WHITE,0.3),1,75),
            (0.50, 0.16, 0.45, 0.001, CYAN_MED,   1,  60),
            (0.58, 0.19, 0.52,-0.001, CYAN_DIM,   1,  45),
            (0.65, 0.22, 0.60, 0.0008,CYAN_FAINT, 1,  35),
            (0.14, 0.04, 0.10, 0.006, WHITE,      1, 160),
            (0.08, 0.025,0.06, 0.010, WHITE_HOT,  1, 200),
        ]
        self.ring_angles=[0.0]*len(self.rings)

    def sw(self): return self.screen.get_width()
    def sh(self): return self.screen.get_height()
    def W(self,pm): return int(self.sw()*pm/1000)
    def H(self,pm): return int(self.sh()*pm/1000)
    def f(self,sz):
        return {8:self.f8,9:self.f9,11:self.f11,13:self.f13,
                16:self.f16,20:self.f20,28:self.f28}.get(sz,self.f11)
    def t(self,s,x,y,txt,col,sz=11):
        r=self.f(sz).render(str(txt),True,col); s.blit(r,(x,y)); return r.get_width()
    def tc(self,s,cx,y,txt,col,sz=11):
        r=self.f(sz).render(str(txt),True,col); s.blit(r,(cx-r.get_width()//2,y))

    def glass_panel(self,surf,rx,ry,rw,rh,col,hover=False):
        try:
            sub = surf.subsurface(pygame.Rect(rx,ry,rw,rh)).copy()
            small = pygame.transform.smoothscale(sub, (max(1, rw//8), max(1, rh//8)))
            blurred = pygame.transform.smoothscale(small, (rw, rh))
            surf.blit(blurred, (rx,ry))
        except ValueError: pass
        alpha=160 if hover else 120
        ps=pygame.Surface((rw,rh),pygame.SRCALPHA)
        ps.fill((*GLASS_BG,alpha))
        surf.blit(ps,(rx,ry))
        gc=CYAN if hover else col
        for off,a in [(2,12),(1,35),(0,140)]:
            pygame.draw.rect(surf,(*gc,a),(rx-off,ry-off,rw+off*2,rh+off*2),1)
        for ex,ey in [(rx,ry),(rx+rw,ry),(rx,ry+rh),(rx+rw,ry+rh)]:
            dx=10 if ex==rx else -10; dy=10 if ey==ry else -10
            pygame.draw.line(surf,gc,(ex,ey),(ex+dx,ey),2)
            pygame.draw.line(surf,gc,(ex,ey),(ex,ey+dy),2)

    def render_bg(self,surf):
        sw,sh=self.sw(),self.sh()
        scaled=pygame.transform.scale(self.galaxy_surf,(sw,sh))
        surf.blit(scaled,(0,0))
        ox=(self.mx/sw-0.5); oy=(self.my/sh-0.5)
        for layer in self.star_layers:
            for sx,sy,br,spd,phase in layer:
                ssx=int(sx*sw/self.W0+ox*sw*spd*40)%sw
                ssy=int(sy*sh/self.H0+oy*sh*spd*40)%sh
                tw=br+int(math.sin(self.tick*0.05+phase)*30)
                tw=clamp(tw,15,230)
                surf.set_at((ssx,ssy),(tw,tw,tw))

    def render_vortex(self,surf, mx_norm, my_norm):
        st=self.state; sw,sh=self.sw(),self.sh()
        cx=sw//2 - int(mx_norm * 15); cy=int(sh*0.47) - int(my_norm * 15)
        it=self.tick*0.05; ink=0.65+0.35*math.sin(it) if st.inference else 0.15+0.08*math.sin(it*0.4)
        RS=sh*0.47
        # Use voice bridge emotion color if available
        vortex_col = self._vortex_color_override if self._vortex_color_override else CYAN
        for i,(rx_f,ry_f,tilt,spd,col,w,amax) in enumerate(self.rings):
            self.ring_angles[i]+=spd
        ring_surf=pygame.Surface((sw,sh),pygame.SRCALPHA)
        for i in range(len(self.rings)-1,-1,-1):
            rx_f,ry_f,tilt,spd,col,w,amax=self.rings[i]
            rx=RS*rx_f*2.1; ry=RS*ry_f*2.1; phase=self.ring_angles[i]; pts=[]
            n_pts=200
            for k in range(n_pts+1):
                a=k*2*math.pi/n_pts+phase*0.05
                ex=rx*math.cos(a); ey=ry*math.sin(a); ey2=ey*math.cos(tilt); ez=ey*math.sin(tilt)
                depth=(ez/(ry*math.sin(tilt)+1e-6)+1)*0.5 if abs(ry*math.sin(tilt))>1 else 0.5
                depth=clamp(depth,0,1); pxv=int(cx+ex); pyv=int(cy+ey2)
                if k<n_pts:
                    a_val=int(amax*depth*ink); c2=lerp_c(dim_c(col,100),col,depth)
                    c2=lerp_c(c2,WHITE_HOT,ink*0.2)
                    if a_val>6: pts.append((pxv,pyv,a_val,c2,depth))
            if len(pts)>2:
                for k,(pxv,pyv,a_val,c2,depth) in enumerate(pts):
                    if k==0: continue
                    prev=pts[k-1]; mid_a=(a_val+prev[2])//2
                    pygame.draw.line(ring_surf,(*c2,mid_a),(prev[0],prev[1]),(pxv,pyv),w)
                    if depth>0.72 and (k+i*7+self.tick//3)%22==0:
                        sz2=2 if depth>0.88 else 1; alpha_sparkle = min(255, int(a_val*1.4))
                        pygame.draw.circle(ring_surf,(*WHITE_HOT,alpha_sparkle),(pxv,pyv),sz2)
        surf.blit(ring_surf,(0,0))
        lbl_x_l=cx-int(RS*0.65)-self.W(10); lbl_x_r=cx+int(RS*0.55)+self.W(5); lbl_y=cy-8
        self.t(surf,lbl_x_l-self.W(65),lbl_y,"KERNEL_SHELL",CYAN_DIM,11)
        self.t(surf,lbl_x_r,lbl_y,"ML_LOGICAL",CYAN_DIM,11)
        pygame.draw.line(surf,(*CYAN_DIM,40),(lbl_x_l,cy),(lbl_x_r+self.W(55),cy),1)
        core_R=int(sh*0.08); glow_col=lerp_c(vortex_col,WHITE_HOT,ink)
        for gr,ga_b in [(core_R+80,3),(core_R+50,6),(core_R+28,12),(core_R+12,20),(core_R+4,36)]:
            ga=int(ga_b*ink*1.5); gs=pygame.Surface((gr*2+4,gr*2+4),pygame.SRCALPHA)
            pygame.draw.circle(gs,(*glow_col,ga),(gr+2,gr+2),gr); surf.blit(gs,(cx-gr-2,cy-gr-2))
        core_r=int(8+ink*18+math.sin(self.tick*0.22)*4)
        for gr2,ga2 in [(core_r+35,30),(core_r+16,65),(core_r+6,120)]:
            gs3=pygame.Surface((gr2*2+4,gr2*2+4),pygame.SRCALPHA)
            wh=lerp_c(vortex_col,WHITE_HOT,ink); pygame.draw.circle(gs3,(*wh,ga2),(gr2+2,gr2+2),gr2)
            surf.blit(gs3,(cx-gr2-2,cy-gr2-2))
        pygame.draw.circle(surf,WHITE_HOT,(cx,cy),core_r); pygame.draw.circle(surf,WHITE,(cx,cy),max(1,core_r-5))
        self.tc(surf,cx,cy+int(sh*0.37),"COR_SET3",CYAN_DIM,9)
        arc_r=core_R+35; wp=st.warden_pressure; wc=RED if wp>180 else AMBER if wp>120 else GREEN
        pct=wp/255.0
        if pct>0.01:
            rect=(cx-arc_r,cy-arc_r,arc_r*2,arc_r*2); s0=-math.pi/2; s1=s0+2*math.pi*pct
            pygame.draw.arc(surf,dim_c(wc,50),rect,-s1,-s0,5); pygame.draw.arc(surf,wc,rect,-s1,-s0,2)
        if st.inference and self.tick%8==0:
            tx=random.choice([self.W(175),self.W(840)]); ty=random.randint(self.H(200),self.H(520))
            c=random.choice([CYAN,WHITE,AMBER,lerp_c(CYAN,WHITE_HOT,0.5)])
            self.token_parts.append(TokenParticle(cx,cy,tx,ty,c))

    def render_particles(self,surf):
        for p in self.token_parts[:]:
            p.update()
            if p.dead: self.token_parts.remove(p); continue
            alpha=int(p.life*200); ps2=pygame.Surface((p.r*2+2,p.r*2+2),pygame.SRCALPHA)
            pygame.draw.circle(ps2,(*p.col,alpha),(p.r+1,p.r+1),p.r); surf.blit(ps2,(int(p.x)-p.r,int(p.y)-p.r))

    def render_synaptic(self,surf, mx_norm, my_norm):
        px,py=14 + int(mx_norm * 30), 44 + int(my_norm * 30); pw,ph=self.W(195),self.H(530)
        hover=self.hover_left
        if hover: px-=3; py-=3; pw+=6; ph+=6
        self.glass_panel(surf,px,py,pw,ph,CYAN_DIM,hover); self.tc(surf,px+pw//2,py+7,"SYNAPTIC BLUEPRINT",CYAN,11)
        nx_off,ny_off=px+10,py+28; nw,nh=pw-20,int(ph*0.36); pt=self.tick*0.04
        for i,j in self.syn_edges:
            ax=nx_off+int(self.syn_nodes[i][0]*nw); ay=ny_off+int(self.syn_nodes[i][1]*nh)
            bx=nx_off+int(self.syn_nodes[j][0]*nw); by=ny_off+int(self.syn_nodes[j][1]*nh)
            ph2=(self.syn_nodes[i][2]+self.syn_nodes[j][2])*math.pi
            alpha=20+int((math.sin(pt+ph2)*0.5+0.5)*60); pygame.draw.line(surf,(0,alpha,int(alpha*1.7)),(ax,ay),(bx,by),1)
        if self.tick%16==0 and self.syn_edges:
            e=random.choice(self.syn_edges); ax=nx_off+int(self.syn_nodes[e[0]][0]*nw); ay=ny_off+int(self.syn_nodes[e[0]][1]*nh)
            bx=nx_off+int(self.syn_nodes[e[1]][0]*nw); by=ny_off+int(self.syn_nodes[e[1]][1]*nh); self.syn_lightning.append([ax,ay,bx,by,10,CYAN])
        for bolt in self.syn_lightning[:]:
            ax2,ay2,bx2,by2,life,col=bolt
            if life>0:
                mx2=(ax2+bx2)//2+random.randint(-8,8); my2=(ay2+by2)//2+random.randint(-8,8)
                bc=lerp_c(CYAN_DIM,CYAN,life/10.0); pygame.draw.line(surf,bc,(ax2,ay2),(mx2,my2),1); pygame.draw.line(surf,bc,(mx2,my2),(bx2,by2),1); bolt[4]-=1
            else: self.syn_lightning.remove(bolt)
        for i,(nx2,ny2,nz) in enumerate(self.syn_nodes):
            sx2=nx_off+int(nx2*nw); sy2=ny_off+int(ny2*nh); pulse=math.sin(pt+nz*math.pi*4); bright=int((pulse*0.5+0.5)*255); r2=2 if bright>160 else 1
            pygame.draw.circle(surf,lerp_c(CYAN_DIM,CYAN,bright/255.0),(sx2,sy2),r2)
        ry=py+28+nh+14; st=self.state
        def row(label,val,col=CYAN_MED):
            nonlocal ry; self.t(surf,px+8,ry,label,CYAN_DIM,9); self.t(surf,px+8,ry+11,str(val),col,11); ry+=27
        row("STATE",STATE_NAMES[st.node_state],STATE_COLORS[st.node_state])
        wp=st.warden_pressure; wc=RED if wp>180 else AMBER if wp>120 else GREEN; row("WARDEN",f"{wp}/255",wc); row("TOKENS/SEC",f"{st.tokens_per_sec}",GREEN); row("D+ MODE",DPLUS_NAMES[st.dplus_mode],YELLOW); row("MODEL",st.model,CYAN); row("UPTIME",f"{st.uptime_sec//3600:02d}:{(st.uptime_sec%3600)//60:02d}:{st.uptime_sec%60:02d}",WHITE)
        bx2,bw2=px+8,pw-18; pygame.draw.rect(surf,(5,10,25),(bx2,ry,bw2,7)); pygame.draw.rect(surf,CYAN_DIM,(bx2,ry,bw2,7),1); fb=int((bw2-2)*wp/255)
        for pi in range(fb):
            bright = math.sin((pi*0.5 - self.tick*0.3)) * 0.5 + 0.5
            if bright > 0.4 or pi % 4 == 0:
                bc = lerp_c(wc, WHITE_HOT, bright*0.4); pygame.draw.line(surf,bc,(bx2+1+pi,ry+1),(bx2+1+pi,ry+5))
        ry+=14; self.t(surf,px+8,ry,"MEMORY ZONES",CYAN_DIM,9); ry+=12
        for a in st.arenas:
            pct=int(a["used"]*100/a["total"]) if a["total"]>0 else 0
            if pct<50: mc=lerp_c(GREEN,AMBER,pct/50.0)
            elif pct<80: mc=lerp_c(AMBER,ORANGE_C,(pct-50)/30.0)
            else: mc=lerp_c(ORANGE_C,RED_HOT,(pct-80)/20.0)
            self.t(surf,px+8,ry,f"{a['name'][:6]} {pct:3d}%",mc,9); self.draw_heatmap(surf, px+8, ry+10, pw-18, 10, pct, mc); ry+=22

    def render_analyze(self,surf, mx_norm, my_norm):
        sw,sh=self.sw(),self.sh(); pw,ph=self.W(202),self.H(530); px=sw-pw-14 + int(mx_norm * 30); py=44 + int(my_norm * 30); hover=self.hover_right
        if hover: px-=3; py-=3; pw+=6; ph+=6
        self.glass_panel(surf,px,py,pw,ph,CYAN_DIM,hover); self.tc(surf,px+pw//2,py+7,"ANALYZE",CYAN,11)
        globe_r=int(self.H(40))
        for gi,(gx_off,spd_m,gc) in enumerate([(pw//3,1.0,CYAN),(pw*2//3,0.6,lerp_c(CYAN,GREEN,0.6))]):
            gcx=px+gx_off; gcy=py+72; gs=pygame.Surface((globe_r*2+22,globe_r*2+22),pygame.SRCALPHA)
            pygame.draw.circle(gs,(*gc,10),(globe_r+11,globe_r+11),globe_r+11); pygame.draw.circle(gs,(*gc,30),(globe_r+11,globe_r+11),globe_r+4); surf.blit(gs,(gcx-globe_r-11,gcy-globe_r-11))
            for k2 in range(4):
                ang_y2=k2*math.pi/2+self.tick*0.005*spd_m; prev2=None
                for i2 in range(33):
                    a2=i2*2*math.pi/32; x0,y0,z0=math.cos(a2),math.sin(a2),0.0; x1=x0*math.cos(ang_y2)+z0*math.sin(ang_y2); y1=y0; z1=-x0*math.sin(ang_y2)+z0*math.cos(ang_y2); depth=(z1+1)*0.5; alp=int(depth*155); gx2v=gcx+int(x1*globe_r); gy2v=gcy-int(y1*globe_r)
                    if prev2 and alp>18: pygame.draw.line(surf,(*gc,alp),prev2,(gx2v,gy2v),1)
                    prev2=(gx2v,gy2v)
            pygame.draw.circle(surf,gc,(gcx,gcy),3); lbl=["SOMA","SWARM"][gi]; self.tc(surf,gcx,gcy+globe_r+5,lbl,gc,9)
        ry=py+72+globe_r+22; st=self.state; pygame.draw.circle(surf,STATE_COLORS[st.node_state],(px+16,ry+5),5); self.t(surf,px+28,ry,STATE_NAMES[st.node_state],STATE_COLORS[st.node_state],11); ry+=20; pygame.draw.line(surf,CYAN_FAINT,(px+8,ry),(px+pw-8,ry),1); ry+=8
        for label,val,col in [("2.1.0-oo","KERNEL VER",CYAN),("bare-metal","BUILD",CYAN_MED),("x86_64","ARCH",CYAN_MED),("UEFI GOP","BOOT",CYAN_MED),(f"{sum(a['total'] for a in st.arenas)} MB","ARENA",WHITE),(f"{st.swarm_count}/4","NODES",GREEN),(f"{st.tokens_per_sec}","TPS",WHITE),(f"{st.tokens_total:,}","GEN",WHITE)]:
            self.t(surf,px+8,ry,label,WHITE,11); ry+=14; pygame.draw.line(surf,CYAN_FAINT,(px+8,ry),(px+pw-8,ry),1); ry+=6
        ry+=4; self.t(surf,px+8,ry,"EVENTS",CYAN_DIM,9); ry+=13
        for ev in st.events[-3:]: self.t(surf,px+8,ry,ev[:24],CYAN_MED,9); ry+=13
        self.draw_radar(surf, px + pw - 40, py + 120, 30)

    def render_statusbar(self,surf):
        sw,sh=self.sw(),self.sh(); bh=self.H(72); by=sh-bh; st=self.state; ps=pygame.Surface((sw,bh),pygame.SRCALPHA); ps.fill((*GLASS_BG,215)); surf.blit(ps,(0,by)); pygame.draw.line(surf,CYAN_DIM,(0,by),(sw,by),2)
        cols=[("NODE_ZD",f"{st.node_id}",CYAN),("SWARM",f"{st.swarm_count}/4 ACTIVE",GREEN),("DEEP INFERENCE","──",CYAN_MED),("THE ENTERPRISE NETWORK","──",CYAN_MED),(f"WARDEN: {st.warden_pressure}/255","",RED if st.warden_pressure>180 else AMBER if st.warden_pressure>120 else GREEN),(f"D+ {DPLUS_NAMES[st.dplus_mode]}","",YELLOW),(f"UP {st.uptime_sec//3600:02d}:{(st.uptime_sec%3600)//60:02d}:{st.uptime_sec%60:02d}","",WHITE)]; col_w=sw//len(cols)
        for i,(label,val,col) in enumerate(cols):
            cx2=i*col_w+col_w//2; self.tc(surf,cx2,by+7,label,col,9); 
            if val and val!="──": self.tc(surf,cx2,by+20,val,dim_c(col,180),9)
            if i>0: pygame.draw.line(surf,CYAN_FAINT,(i*col_w,by+3),(i*col_w,by+bh-18),1)
        TICKER=("  OO BAREMETAL ENGINE v0.3  ///  UEFI RING-0 GOP  ///  LLM-40M ACTIVE  ///  WARDEN: NOMINAL  ///  D+ POLICY ENGAGED  ///  SOMA-CORE PULSING  ///  INFERENCE LOOP RUNNING  ///  GRAVITATIONAL VORTEX ONLINE  ///  SOMA NEURAL ENGINE  ///  ")
        tick_y=by+bh-17; pygame.draw.line(surf,CYAN_FAINT,(0,tick_y-2),(sw,tick_y-2),1); char_w=7; total_w=len(TICKER)*char_w; self.ticker_off=(self.ticker_off+1)%total_w; r2=self.f9.render(TICKER*3,True,CYAN_DIM); surf.blit(r2,(-self.ticker_off,tick_y))
        inp_w=self.W(300); inp_x=sw-inp_w-14; inp_y=by+bh-17; pygame.draw.rect(surf,(3,8,22),(inp_x,inp_y,inp_w,15)); pygame.draw.rect(surf,CYAN_DIM,(inp_x,inp_y,inp_w,15),1); prompt=f"> {st.input_buf}"; 
        if self.tick%60<30: prompt+="_"
        self.t(surf,inp_x+4,inp_y+2,prompt,CYAN_MED,9); wave_cx=self.W(100); wave_cy=by-self.H(60); base_r=self.H(30); c_label=GREEN if st.voice_active else CYAN_DIM; self.tc(surf,wave_cx,wave_cy+base_r+20,"VOICE CYMATICS",c_label,9); pts_out=[]
        for i in range(64):
            amp=st.voice_wave[i]; bump=(amp/255.0)*self.H(25); r_out=base_r+bump; ang=i*math.pi*2/64; x_in=wave_cx+base_r*math.cos(ang); y_in=wave_cy+base_r*math.sin(ang); x_out=wave_cx+r_out*math.cos(ang); y_out=wave_cy+r_out*math.sin(ang); c_line=GREEN if st.voice_active else CYAN_DIM; pygame.draw.line(surf,c_line,(x_in,y_in),(x_out,y_out),1); pts_out.append((x_out,y_out))
        if len(pts_out)>1: pygame.draw.polygon(surf,c_line,pts_out,1)

    def render_header(self,surf):
        sw=self.sw(); st=self.state; hs=pygame.Surface((sw,42),pygame.SRCALPHA); hs.fill((*GLASS_BG,130)); surf.blit(hs,(0,0)); pygame.draw.line(surf,CYAN_DIM,(0,41),(sw,41),1); self.tc(surf,sw//2,8,"STREAM OF CONSCIOUSNESS",CYAN,20); self.tc(surf,sw//2,30,"- INFERENCE ACTIVE",CYAN_DIM,9); self.t(surf,16,8,"v0.3-alpha",CYAN_DIM,9); self.t(surf,16,20,"SOMA-CORE",CYAN_DIM,9); tc=f"TICK:{self.tick:06d}"; self.t(surf,sw-self.f9.size(tc)[0]-14,8,tc,CYAN_DIM,9); self.t(surf,sw-80,20,"60Hz",CYAN_DIM,9)

    def _render_watermarks(self, surf):
        """Gros filigranes: LLM-BAREMETAL en haut gauche, OO en bas droite."""
        sw, sh = surf.get_size()

        # ── LLM-BAREMETAL (haut gauche, semi-transparent) ─────────────────────
        try:
            wf_big = pygame.font.SysFont("Courier New", 52, bold=True)
        except Exception:
            wf_big = self.f28

        text_llm = "LLM-BAREMETAL"
        r_llm = wf_big.render(text_llm, True, CYAN)
        ws_llm = pygame.Surface(r_llm.get_size(), pygame.SRCALPHA)
        ws_llm.blit(r_llm, (0, 0))
        ws_llm.set_alpha(22 + int(8 * math.sin(self.tick * 0.03)))
        surf.blit(ws_llm, (18, 52))

        # Chromatic aberration (red + blue offsets)
        r_llm_r = wf_big.render(text_llm, True, (200, 0, 0))
        r_llm_b = wf_big.render(text_llm, True, (0, 0, 200))
        ws_r = pygame.Surface(r_llm_r.get_size(), pygame.SRCALPHA)
        ws_b = pygame.Surface(r_llm_b.get_size(), pygame.SRCALPHA)
        ws_r.blit(r_llm_r, (0, 0)); ws_r.set_alpha(8)
        ws_b.blit(r_llm_b, (0, 0)); ws_b.set_alpha(8)
        surf.blit(ws_r, (21, 52))
        surf.blit(ws_b, (15, 55))

        # Second line smaller
        try:
            wf_med = pygame.font.SysFont("Courier New", 20, bold=True)
        except Exception:
            wf_med = self.f20
        r_sub = wf_med.render("Operating Organism  ·  Bare-Metal Intelligence", True, CYAN_MED)
        ws_sub = pygame.Surface(r_sub.get_size(), pygame.SRCALPHA)
        ws_sub.blit(r_sub, (0, 0))
        ws_sub.set_alpha(18)
        surf.blit(ws_sub, (18, 107))

        # ── OO (bas droite, grand, pulsant) ───────────────────────────────────
        try:
            wf_oo = pygame.font.SysFont("Courier New", 120, bold=True)
        except Exception:
            wf_oo = self.f28
        pulse = 0.5 + 0.5 * math.sin(self.tick * 0.04)
        alpha_oo = int(15 + 12 * pulse)
        r_oo = wf_oo.render("OO", True, CYAN)
        ws_oo = pygame.Surface(r_oo.get_size(), pygame.SRCALPHA)
        ws_oo.blit(r_oo, (0, 0))
        ws_oo.set_alpha(alpha_oo)
        ox = sw - r_oo.get_width() - 22
        oy = sh - r_oo.get_height() - 52
        surf.blit(ws_oo, (ox, oy))

        # OO chromatic layers
        r_oo_r = wf_oo.render("OO", True, (180, 0, 0))
        r_oo_b = wf_oo.render("OO", True, (0, 0, 180))
        ws_oo_r = pygame.Surface(r_oo_r.get_size(), pygame.SRCALPHA); ws_oo_r.blit(r_oo_r, (0,0)); ws_oo_r.set_alpha(6)
        ws_oo_b = pygame.Surface(r_oo_b.get_size(), pygame.SRCALPHA); ws_oo_b.blit(r_oo_b, (0,0)); ws_oo_b.set_alpha(6)
        surf.blit(ws_oo_r, (ox + 4, oy - 2))
        surf.blit(ws_oo_b, (ox - 4, oy + 2))

        # Version label sous OO
        r_ver = wf_med.render("v0.1-alpha", True, CYAN_DIM)
        ws_ver = pygame.Surface(r_ver.get_size(), pygame.SRCALPHA)
        ws_ver.blit(r_ver, (0, 0)); ws_ver.set_alpha(14)
        surf.blit(ws_ver, (sw - r_ver.get_width() - 22, sh - 46))

    def render_cursor(self, surf):
        mx,my=self.mx,self.my; sc=STATE_COLORS[self.state.node_state]; sw,sh=self.sw(),self.sh()
        for sx,sy,br,spd,phase in self.star_layers[2]:
            ssx=int(sx*sw/self.W0+(self.mx/sw-0.5)*sw*spd*40)%sw; ssy=int(sy*sh/self.H0+(self.my/sh-0.5)*sh*spd*40)%sh; dx=mx-ssx; dy=my-ssy; d=math.hypot(dx,dy)
            if 6<d<85:
                pull=(85-d)/85.0; ex=ssx+int(dx*pull*0.35); ey=ssy+int(dy*pull*0.35); pygame.draw.line(surf,(*CYAN_DIM,int(pull*50)),(ssx,ssy),(ex,ey),1)
        for p1,p2 in [((mx-14,my),(mx-6,my)),((mx+6,my),(mx+14,my)),((mx,my-14),(mx,my-6)),((mx,my+6),(mx,my+14))]: pygame.draw.line(surf,sc,p1,p2,1)
        pygame.draw.circle(surf,sc,(mx,my),2); gs=pygame.Surface((44,44),pygame.SRCALPHA); pygame.draw.circle(gs,(*CYAN_DIM,70),(22,22),20,1); surf.blit(gs,(mx-22,my-22))
        if self.hover_sphere: gs2=pygame.Surface((64,64),pygame.SRCALPHA); pygame.draw.circle(gs2,(*sc,45),(32,32),30,1); surf.blit(gs2,(mx-32,my-32))

    def render_glitch(self,surf):
        if self.state.glitch<=0: return
        sw,sh=self.sw(),self.sh()
        for _ in range(5):
            gy=random.randint(0,sh-7); gsh=random.randint(2,7); gh=min(gsh,sh-gy)
            if gh<=0: continue
            region=surf.subsurface(pygame.Rect(0,gy,sw,gh)); tmp=region.copy(); surf.blit(tmp,(random.randint(-22,22),gy))
        tint=pygame.Surface((sw,sh),pygame.SRCALPHA); tint.fill((200,0,0,18)); surf.blit(tint,(0,0))

    def render_boot_sequence(self, surf):
        sw, sh = self.sw(), self.sh()
        surf.fill(DEEPSPACE)
        cx, cy = sw // 2, sh // 2 - 50

        if self.tick < 60:
            # Matrix rain fills background at reduced alpha
            self.render_matrix_rain(surf, alpha=90)

            # Dark overlay to read text on top of rain
            overlay = pygame.Surface((sw, sh), pygame.SRCALPHA)
            overlay.fill((0, 0, 8, 160))
            surf.blit(overlay, (0, 0))

            # DIOP_MIND header with chromatic aberration
            self.draw_title_glitch(surf, sw // 2, sh // 2 - 140,
                                   "DIOP_MIND OPERATING ORGANISM", CYAN, 22, intensity=3)

            # Typewriter lines
            ty = sh // 2 - 90
            for (start_tick, text, col, size) in self.boot_lines:
                if self.tick > start_tick:
                    # Reveal chars progressively
                    revealed = min(len(text), (self.tick - start_tick) * 3)
                    partial = text[:revealed]
                    self.t(surf, sw // 2 - 280, ty, partial, col, size)
                    if revealed < len(text) and (self.tick // 3) % 2 == 0:
                        pygame.draw.rect(surf, WHITE,
                                        (sw//2 - 280 + self.f(size).size(partial)[0] + 2, ty, 7, size))
                ty += size + 6

        elif self.tick < 120:
            # Galaxy BG fades in
            self.render_bg(surf)
            t = (self.tick - 60) / 60.0
            fade_overlay = pygame.Surface((sw, sh), pygame.SRCALPHA)
            fade_overlay.fill((0, 0, 10, int(220 * (1 - t))))
            surf.blit(fade_overlay, (0, 0))

            # Expanding rings from center
            max_r = self.H(140)
            current_r = int(t * max_r)
            for ri in range(3):
                r_off = ri * 30
                if current_r > r_off:
                    alpha = max(0, 180 - ri * 60)
                    col = lerp_c(CYAN, WHITE_HOT, ri / 3.0)
                    ring_s = pygame.Surface((sw, sh), pygame.SRCALPHA)
                    pygame.draw.circle(ring_s, (*col, alpha),
                                       (cx, cy), current_r - r_off, 2)
                    surf.blit(ring_s, (0, 0))

            # Progress bar with glow
            bar_w, bar_h = self.W(500), self.H(12)
            bar_x, bar_y = sw // 2 - bar_w // 2, self.H(820)
            # Glow
            glow_s = pygame.Surface((bar_w + 20, bar_h + 20), pygame.SRCALPHA)
            pygame.draw.rect(glow_s, (*CYAN, 30), (0, 0, bar_w + 20, bar_h + 20), border_radius=8)
            surf.blit(glow_s, (bar_x - 10, bar_y - 10))
            # Bar
            pygame.draw.rect(surf, CYAN_DIM, (bar_x, bar_y, bar_w, bar_h), 1, border_radius=3)
            fill_w = int(t * (bar_w - 4))
            if fill_w > 2:
                fill_col = lerp_c(CYAN_MED, WHITE_HOT, t)
                pygame.draw.rect(surf, fill_col,
                                 (bar_x + 2, bar_y + 2, fill_w, bar_h - 4), border_radius=2)
            self.draw_title_glitch(surf, sw // 2, bar_y + bar_h + 12,
                                   "LOADING NEURAL WEIGHTS", WHITE, 11)

        elif self.tick < 180:
            self.render_bg(surf)
            self.render_vortex(surf, 0, 0)

            # Energy waves from core
            self.render_energy_waves(surf, cx, cy, self.state)

            # Wireframes grow in
            progress = (self.tick - 120) / 60.0
            panels = [
                (CYAN_DIM,  self.W(5),             44,               self.W(270), self.H(530)),
                (MAGENTA,   sw-self.W(270)-14,      44,               self.W(270), self.H(530)),
                (CYAN,      sw//2 - self.W(450),    sh - self.H(265), self.W(900), self.H(195)),
            ]
            for col, px, py, pw, ph in panels:
                # Animate height from top down
                animated_h = int(ph * progress)
                # Glow border
                glow_s = pygame.Surface((pw + 8, animated_h + 8), pygame.SRCALPHA)
                pygame.draw.rect(glow_s, (*col, 30), (0, 0, pw + 8, animated_h + 8), border_radius=4)
                surf.blit(glow_s, (px - 4, py - 4))
                # Actual wireframe
                pygame.draw.rect(surf, col, (px, py, pw, animated_h), 1)
                # Corner accents
                acc = 12
                pygame.draw.line(surf, WHITE, (px, py), (px + acc, py), 2)
                pygame.draw.line(surf, WHITE, (px, py), (px, py + acc), 2)
                pygame.draw.line(surf, WHITE, (px + pw, py), (px + pw - acc, py), 2)

            # SYSTEM ONLINE flash with chromatic aberration
            if self.tick % 20 < 10:
                self.draw_title_glitch(surf, sw // 2, cy + self.H(200),
                                       "SYSTEM ONLINE", WHITE, 28, intensity=4)

        # CRT scanlines on every boot frame
        self.render_scanlines(surf)

    def run(self):
        surf=pygame.Surface((self.W0,self.H0))
        running=True
        while running:
            for ev in pygame.event.get():
                if ev.type==pygame.QUIT: running=False
                elif ev.type==pygame.KEYDOWN:
                    if ev.key==pygame.K_ESCAPE: running=False
                    elif ev.key==pygame.K_F11:
                        self.fullscreen=not self.fullscreen
                        if self.fullscreen: self.screen=pygame.display.set_mode((0,0),pygame.FULLSCREEN)
                        else: self.screen=pygame.display.set_mode((self.W0,self.H0),pygame.RESIZABLE)
                    elif ev.key==pygame.K_RETURN:
                        if self.state.input_buf: self.state.response_lines.append(f"> {self.state.input_buf}"); self.state.input_buf=""
                    elif ev.key==pygame.K_BACKSPACE: self.state.input_buf=self.state.input_buf[:-1]
                    elif ev.unicode and len(self.state.input_buf)<60: self.state.input_buf+=ev.unicode
                elif ev.type==pygame.MOUSEMOTION: self.mx,self.my=ev.pos
                elif ev.type==pygame.VIDEORESIZE: self.screen=pygame.display.set_mode((ev.w,ev.h),pygame.RESIZABLE)

            # Tick simulation + voice bridge overlay
            self.state.update(self.tick)
            vb = self._vbridge.get()
            self.state.apply_voice_bridge(vb)
            # Override vortex color from emotion
            ec = vb.get("vortex_color")
            if ec: self._vortex_color_override = tuple(ec)
            else:  self._vortex_color_override = None

            sw,sh=self.screen.get_size()
            if surf.get_size()!=(sw,sh): surf=pygame.Surface((sw,sh))
            mx_norm = (self.mx / sw - 0.5) * 2.0; my_norm = (self.my / sh - 0.5) * 2.0
            if self.tick < 180:
                self.render_boot_sequence(surf)
            else:
                self.render_bg(surf)
                self.render_aurora(surf)                   # Aurora derrière tout
                sw2, sh2 = surf.get_size()
                cxv2 = sw2//2 - int(mx_norm*15)
                cyv2 = int(sh2*0.47) - int(my_norm*15)
                self.render_ghost_panels(surf, mx_norm, my_norm)
                self.render_energy_waves(surf, cxv2, cyv2, self.state)
                self.render_singularity(surf, cxv2, cyv2, self.state)
                # Cymatics en halo autour de l'œil
                self.render_cymatics(surf, cxv2, cyv2, self.H(90), self.state)
                self.render_data_streams(surf)
                self.render_particles(surf)
                pw_l=self.W(195); pw_r=self.W(202); ph_p=self.H(530)
                px_l, py_l = 14 + int(mx_norm * 30), 44 + int(my_norm * 30)
                px_r, py_r = sw2-pw_r-14 + int(mx_norm * 30), 44 + int(my_norm * 30)
                self.hover_left=(px_l<=self.mx<=px_l+pw_l and py_l<=self.my<=py_l+ph_p)
                self.hover_right=(px_r<=self.mx<=px_r+pw_r and py_r<=self.my<=py_r+ph_p)
                self.render_synaptic(surf, mx_norm, my_norm)
                self.render_analyze(surf, mx_norm, my_norm)
                # DNA Helix — panneau gauche haut
                self.render_dna_helix(surf,
                    px_l + pw_l + self.W(20),
                    self.H(60), self.H(240), self.state)
                # Vector Scope — panneau droit bas intérieur
                self.render_vector_scope(surf,
                    px_r - self.W(55),
                    int(sh2 * 0.70), self.H(45), self.state)
                # Lissajous centré bas
                lis_cx = sw2 // 2; lis_cy = int(sh2 * 0.82)
                self.render_lissajous(surf, lis_cx, lis_cy, self.W(200), self.H(120), self.state)
                # Token stream au centre au-dessus de la status bar
                self.render_token_stream(surf,
                    self.W(215), int(sh2 * 0.68),
                    self.W(570), self.H(95), self.state)
                self.render_statusbar(surf)
                self.render_header(surf)
                self.render_glitch(surf)
                self.render_scanlines(surf)
                # ── Gros filigranes OO + LLM-BAREMETAL ───────────────────────
                self._render_watermarks(surf)

            
            self.render_cursor(surf)

            self.screen.blit(surf,(0,0))
            
            pygame.display.flip()
            self.clock.tick(60)
            self.tick+=1
            # Save screenshots for the user
            if self.tick in (30, 90, 160, 250, 320, 420):
                pygame.image.save(self.screen, f"screenshot_{self.tick}.png")
            if self.tick == 430:
                running = False
        self._vbridge.stop()
        pygame.quit()

if __name__=="__main__":
    SomaHUD().run()
