#!/usr/bin/env python3
import json
import math
import os
import sys
import time
from datetime import datetime, timezone

try:
    import pygame
except ImportError:
    print("Install pygame: pip install pygame")
    sys.exit(1)

BG = (10, 14, 28)
PANEL = (16, 24, 44)
ACCENT = (0, 220, 170)
ACCENT2 = (240, 180, 70)
TEXT = (232, 238, 255)
MUTED = (140, 160, 190)

PROFILE_PRESETS = ["general", "enfant", "senior", "diaspora"]
GUARDIAN_PRESET_CYCLE = ["safe", "balanced", "strict"]
GUARDIAN_PRESET_STYLE = {
    "safe": {
        "color": (40, 200, 120),
        "fr": "VIGILANCE SAFE",
        "en": "SAFE VIGILANCE",
    },
    "balanced": {
        "color": (240, 180, 70),
        "fr": "VIGILANCE BALANCED",
        "en": "BALANCED VIGILANCE",
    },
    "strict": {
        "color": (250, 90, 90),
        "fr": "VIGILANCE STRICT",
        "en": "STRICT VIGILANCE",
    },
}
GUARDIAN_ALERT_ACTIVE_LABEL = {
    "fr": "ALERTE ACTIVE",
    "en": "ALERT ACTIVE",
}
GUARDIAN_PRESET_BY_PROFILE = {
    "safe": {
        "general": {"idle_threshold_minutes": 25, "reminder_window_minutes": 45},
        "enfant": {"idle_threshold_minutes": 15, "reminder_window_minutes": 60},
        "senior": {"idle_threshold_minutes": 20, "reminder_window_minutes": 75},
        "diaspora": {"idle_threshold_minutes": 35, "reminder_window_minutes": 45},
    },
    "balanced": {
        "general": {"idle_threshold_minutes": 20, "reminder_window_minutes": 30},
        "enfant": {"idle_threshold_minutes": 10, "reminder_window_minutes": 45},
        "senior": {"idle_threshold_minutes": 15, "reminder_window_minutes": 60},
        "diaspora": {"idle_threshold_minutes": 30, "reminder_window_minutes": 30},
    },
    "strict": {
        "general": {"idle_threshold_minutes": 10, "reminder_window_minutes": 20},
        "enfant": {"idle_threshold_minutes": 5, "reminder_window_minutes": 30},
        "senior": {"idle_threshold_minutes": 10, "reminder_window_minutes": 40},
        "diaspora": {"idle_threshold_minutes": 20, "reminder_window_minutes": 20},
    },
}
VOICE_BRIDGE_FILE = os.path.join(os.path.dirname(__file__), "oo_voice_state.json")
PROFILE_LABELS = {
    "fr": {
        "general": "famille",
        "enfant": "famille enfant",
        "senior": "famille senior",
        "diaspora": "famille diaspora",
    },
    "en": {
        "general": "family",
        "enfant": "child-focused family",
        "senior": "senior-focused family",
        "diaspora": "diaspora family",
    },
}
VOICE_PROMPTS = {
    "fr": {
        "greet_morning": "Bonjour {profile}.",
        "greet_afternoon": "Bon apres-midi {profile}.",
        "greet_evening": "Bonsoir {profile}.",
        "greet_night": "Bonne nuit {profile}.",
        "tone_soft": "Voix douce.",
        "tone_firm": "Voix ferme.",
        "voice_on": "Guide vocal active.",
        "voice_off": "Guide vocal desactive.",
        "lang_fr": "Langue francaise activee.",
        "lang_en": "Langue anglaise activee.",
        "profile_changed": "Profil {profile} active.",
        "mode_changed": "Mode foyer {mode}.",
        "member_added": "Un membre a ete ajoute.",
        "routine_added": "Une routine a ete ajoutee.",
        "quickstart_next": "Etape suivante du quickstart.",
        "quickstart_done": "Premiere vie complete. Keurgui est pret.",
        "guardian_on": "Mode guardian active.",
        "guardian_off": "Mode guardian desactive.",
        "guardian_alert": "Alerte guardian: {alert}",
        "guardian_preset_applied": "Preset guardian {preset} applique.",
        "threshold_idle_updated": "Seuil inactivite {minutes} minutes.",
        "threshold_reminder_updated": "Fenetre rappel routine {minutes} minutes.",
        "alert_child_urgent": "mode urgence avec profil enfant",
        "alert_senior_missing": "profil senior sans routine active",
        "alert_diaspora_missing": "profil diaspora sans membres admin",
        "alert_idle": "aucune interaction depuis {minutes} minutes",
        "alert_routine_due": "routine {routine} a {heure}",
    },
    "en": {
        "greet_morning": "Good morning {profile}.",
        "greet_afternoon": "Good afternoon {profile}.",
        "greet_evening": "Good evening {profile}.",
        "greet_night": "Good night {profile}.",
        "tone_soft": "Soft voice.",
        "tone_firm": "Firm voice.",
        "voice_on": "Voice guide enabled.",
        "voice_off": "Voice guide disabled.",
        "lang_fr": "French language enabled.",
        "lang_en": "English language enabled.",
        "profile_changed": "Profile {profile} enabled.",
        "mode_changed": "Home mode set to {mode}.",
        "member_added": "A member has been added.",
        "routine_added": "A routine has been added.",
        "quickstart_next": "Moving to the next quickstart step.",
        "quickstart_done": "First life complete. Keurgui is ready.",
        "guardian_on": "Guardian mode enabled.",
        "guardian_off": "Guardian mode disabled.",
        "guardian_alert": "Guardian alert: {alert}",
        "guardian_preset_applied": "Guardian preset {preset} applied.",
        "threshold_idle_updated": "Idle threshold set to {minutes} minutes.",
        "threshold_reminder_updated": "Routine reminder window set to {minutes} minutes.",
        "alert_child_urgent": "urgent mode while child profile is active",
        "alert_senior_missing": "senior profile has no active routine",
        "alert_diaspora_missing": "diaspora profile has no admin member",
        "alert_idle": "no interaction for {minutes} minutes",
        "alert_routine_due": "routine {routine} at {heure}",
    },
}


def utc_now_iso():
    return datetime.now(timezone.utc).isoformat()


def load_state(path):
    base = {
        "nom_foyer": "Keurgui Maison",
        "mode_foyer": "normal",
        "langue": "fr",
        "onboarding_profile": "general",
        "voice_guide_enabled": False,
        "guardian_mode_enabled": False,
        "guardian_idle_threshold_minutes": 20,
        "guardian_reminder_window_minutes": 30,
        "guardian_last_interaction_at": "",
        "guardian_last_alert": "",
        "guardian_profile_preset_modes": {
            "general": "balanced",
            "enfant": "balanced",
            "senior": "balanced",
            "diaspora": "balanced",
        },
        "guardian_profile_thresholds": {
            "general": {"idle_threshold_minutes": 20, "reminder_window_minutes": 30},
            "enfant": {"idle_threshold_minutes": 10, "reminder_window_minutes": 45},
            "senior": {"idle_threshold_minutes": 15, "reminder_window_minutes": 60},
            "diaspora": {"idle_threshold_minutes": 30, "reminder_window_minutes": 30},
        },
        "first_life_completed": False,
        "membres": [],
        "routines": [],
        "futures": [],
        "quickstart": {
            "enabled": True,
            "step": 1,
            "steps_total": 4,
            "completed": False,
            "last_action": "",
        },
    }

    if not os.path.exists(path):
        return base

    with open(path, "r", encoding="utf-8") as f:
        state = json.load(f)

    for k, v in base.items():
        if k not in state:
            state[k] = v

    if not isinstance(state.get("quickstart"), dict):
        state["quickstart"] = base["quickstart"]

    for k, v in base["quickstart"].items():
        if k not in state["quickstart"]:
            state["quickstart"][k] = v

    if not isinstance(state.get("guardian_profile_thresholds"), dict):
        state["guardian_profile_thresholds"] = base["guardian_profile_thresholds"]
    for profile_key, profile_defaults in base["guardian_profile_thresholds"].items():
        profile_cfg = state["guardian_profile_thresholds"].get(profile_key)
        if not isinstance(profile_cfg, dict):
            state["guardian_profile_thresholds"][profile_key] = dict(profile_defaults)
            continue
        for threshold_key, threshold_value in profile_defaults.items():
            if threshold_key not in profile_cfg:
                profile_cfg[threshold_key] = threshold_value

    if not isinstance(state.get("guardian_profile_preset_modes"), dict):
        state["guardian_profile_preset_modes"] = base["guardian_profile_preset_modes"]
    for profile_key, default_mode in base["guardian_profile_preset_modes"].items():
        current_mode = state["guardian_profile_preset_modes"].get(profile_key)
        if current_mode not in GUARDIAN_PRESET_CYCLE:
            state["guardian_profile_preset_modes"][profile_key] = default_mode

    return state


def save_state(path, state):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    state["updated_at"] = utc_now_iso()
    with open(path, "w", encoding="utf-8") as f:
        json.dump(state, f, ensure_ascii=True, indent=2)


def parse_iso_ts(value):
    if not value or not isinstance(value, str):
        return None
    try:
        return datetime.fromisoformat(value.replace("Z", "+00:00"))
    except ValueError:
        return None


def parse_hhmm(value):
    if not isinstance(value, str) or ":" not in value:
        return None
    parts = value.split(":", 1)
    try:
        hh = int(parts[0])
        mm = int(parts[1])
    except ValueError:
        return None
    if hh < 0 or hh > 23 or mm < 0 or mm > 59:
        return None
    return (hh, mm)


def mark_interaction(state):
    state["guardian_last_interaction_at"] = utc_now_iso()


def quickstart_next(state):
    qs = state["quickstart"]
    if qs.get("completed"):
        return

    step = int(qs.get("step", 1))
    total = int(qs.get("steps_total", 4))
    if step < total:
        qs["step"] = step + 1
        qs["last_action"] = f"step_{step}_done"
    else:
        qs["step"] = total
        qs["completed"] = True
        state["first_life_completed"] = True
        qs["last_action"] = "first_life_completed"


def cycle_profile(state):
    current = state.get("onboarding_profile", "general")
    idx = PROFILE_PRESETS.index(current) if current in PROFILE_PRESETS else 0
    state["onboarding_profile"] = PROFILE_PRESETS[(idx + 1) % len(PROFILE_PRESETS)]
    state["quickstart"]["last_action"] = f"profile={state['onboarding_profile']}"


def emit_voice_prompt(state, text):
    mode = state.get("mode_foyer", "normal")
    emotion = "CALM" if mode == "calme" else "ALERT" if mode == "urgence" else "CURIOUS"
    payload = {
        "emotion": emotion,
        "inference": False,
        "response": text,
        "waveform": [128] * 64,
    }
    try:
        with open(VOICE_BRIDGE_FILE, "w", encoding="utf-8") as f:
            json.dump(payload, f, ensure_ascii=True)
    except OSError:
        pass


def get_lang_key(state):
    lang = state.get("langue", "fr")
    return "en" if lang == "en" else "fr"


def current_day_part():
    hour = datetime.now().hour
    if 5 <= hour < 12:
        return "morning"
    if 12 <= hour < 18:
        return "afternoon"
    if 18 <= hour < 22:
        return "evening"
    return "night"


def localized_profile_label(state):
    lang_key = get_lang_key(state)
    profile = state.get("onboarding_profile", "general")
    labels = PROFILE_LABELS.get(lang_key, PROFILE_LABELS["fr"])
    return labels.get(profile, labels["general"])


def guardian_thresholds_for_profile(state):
    profile = state.get("onboarding_profile", "general")
    profile_cfg = state.get("guardian_profile_thresholds", {}).get(profile, {})
    idle_threshold = int(profile_cfg.get("idle_threshold_minutes", state.get("guardian_idle_threshold_minutes", 20)))
    reminder_window = int(profile_cfg.get("reminder_window_minutes", state.get("guardian_reminder_window_minutes", 30)))
    return idle_threshold, reminder_window


def guardian_preset_for_profile(state):
    profile = state.get("onboarding_profile", "general")
    mode = state.get("guardian_profile_preset_modes", {}).get(profile, "balanced")
    return mode if mode in GUARDIAN_PRESET_CYCLE else "balanced"


def apply_guardian_preset_for_profile(state, preset):
    if preset not in GUARDIAN_PRESET_CYCLE:
        preset = "balanced"
    profile = state.get("onboarding_profile", "general")
    thresholds = state.setdefault("guardian_profile_thresholds", {})
    profile_thresholds = thresholds.setdefault(profile, {})
    preset_values = GUARDIAN_PRESET_BY_PROFILE.get(preset, GUARDIAN_PRESET_BY_PROFILE["balanced"]).get(
        profile,
        GUARDIAN_PRESET_BY_PROFILE["balanced"]["general"],
    )
    profile_thresholds["idle_threshold_minutes"] = int(preset_values["idle_threshold_minutes"])
    profile_thresholds["reminder_window_minutes"] = int(preset_values["reminder_window_minutes"])
    preset_modes = state.setdefault("guardian_profile_preset_modes", {})
    preset_modes[profile] = preset
    return preset


def cycle_guardian_preset_for_profile(state):
    current = guardian_preset_for_profile(state)
    idx = GUARDIAN_PRESET_CYCLE.index(current) if current in GUARDIAN_PRESET_CYCLE else 1
    next_preset = GUARDIAN_PRESET_CYCLE[(idx + 1) % len(GUARDIAN_PRESET_CYCLE)]
    return apply_guardian_preset_for_profile(state, next_preset)


def adjust_profile_threshold(state, field, delta, min_value, max_value):
    profile = state.get("onboarding_profile", "general")
    thresholds = state.setdefault("guardian_profile_thresholds", {})
    profile_cfg = thresholds.setdefault(profile, {})

    if field == "idle_threshold_minutes":
        fallback = state.get("guardian_idle_threshold_minutes", 20)
    else:
        fallback = state.get("guardian_reminder_window_minutes", 30)

    current = int(profile_cfg.get(field, fallback))
    updated = max(min_value, min(max_value, current + delta))
    profile_cfg[field] = updated
    return updated


def voice_text(state, key, **kwargs):
    lang_key = get_lang_key(state)
    table = VOICE_PROMPTS.get(lang_key, VOICE_PROMPTS["fr"])
    msg = table.get(key, key)
    if kwargs:
        return msg.format(**kwargs)
    return msg


def maybe_emit_voice(state, key, **kwargs):
    if not state.get("voice_guide_enabled"):
        return
    mode = state.get("mode_foyer", "normal")
    tone_key = "tone_firm" if mode == "urgence" else "tone_soft"
    day_part = current_day_part()
    greeting_key = f"greet_{day_part}"
    greeting = voice_text(state, greeting_key, profile=localized_profile_label(state))
    prefix = voice_text(state, tone_key)
    body = voice_text(state, key, **kwargs)
    emit_voice_prompt(state, f"{greeting} {prefix} {body}")


def guardian_alerts(state):
    profile = state.get("onboarding_profile", "general")
    mode = state.get("mode_foyer", "normal")
    routines = state.get("routines", [])
    members = state.get("membres", [])
    alerts = []

    if profile == "enfant" and mode == "urgence":
        alerts.append(voice_text(state, "alert_child_urgent"))

    has_active_routine = any(bool(r.get("actif", False)) for r in routines)
    if profile == "senior" and not has_active_routine:
        alerts.append(voice_text(state, "alert_senior_missing"))

    has_admin = any(m.get("role") == "admin" for m in members)
    if profile == "diaspora" and not has_admin:
        alerts.append(voice_text(state, "alert_diaspora_missing"))

    idle_threshold, reminder_window = guardian_thresholds_for_profile(state)
    last_interaction = parse_iso_ts(state.get("guardian_last_interaction_at", ""))
    if last_interaction is not None:
        idle_minutes = int((datetime.now(timezone.utc) - last_interaction).total_seconds() // 60)
        if idle_minutes >= idle_threshold:
            alerts.append(voice_text(state, "alert_idle", minutes=idle_minutes))

    now_local = datetime.now()
    now_minutes = now_local.hour * 60 + now_local.minute
    next_routine = None
    best_delta = None
    for routine in routines:
        if not bool(routine.get("actif", False)):
            continue
        parsed = parse_hhmm(routine.get("heure", ""))
        if parsed is None:
            continue
        routine_minutes = parsed[0] * 60 + parsed[1]
        delta = routine_minutes - now_minutes
        if 0 <= delta <= reminder_window and (best_delta is None or delta < best_delta):
            best_delta = delta
            next_routine = routine

    if next_routine is not None:
        alerts.append(
            voice_text(
                state,
                "alert_routine_due",
                routine=next_routine.get("nom", "Routine"),
                heure=next_routine.get("heure", "--:--"),
            )
        )

    return alerts


def draw_panel(surface, rect, title, font_title, font_text, lines):
    pygame.draw.rect(surface, PANEL, rect, border_radius=16)
    pygame.draw.rect(surface, ACCENT, rect, width=2, border_radius=16)
    t = font_title.render(title, True, ACCENT2)
    surface.blit(t, (rect.x + 14, rect.y + 10))
    y = rect.y + 44
    for line in lines:
        txt = font_text.render(line, True, TEXT)
        surface.blit(txt, (rect.x + 14, y))
        y += 24


def draw_guardian_vigilance_badge(surface, rect, font_small, state, preset):
    style = GUARDIAN_PRESET_STYLE.get(preset, GUARDIAN_PRESET_STYLE["balanced"])
    lang_key = get_lang_key(state)
    label = style.get(lang_key, style["fr"])
    badge_color = style["color"]
    badge_rect = pygame.Rect(rect.x + 14, rect.y + 12, 260, 28)
    pygame.draw.rect(surface, badge_color, badge_rect, border_radius=10)
    text = font_small.render(label, True, (8, 12, 20))
    surface.blit(text, (badge_rect.x + 10, badge_rect.y + 6))


def draw_guardian_vigilance_badge_animated(surface, rect, font_small, state, preset, has_alert):
    style = GUARDIAN_PRESET_STYLE.get(preset, GUARDIAN_PRESET_STYLE["balanced"])
    lang_key = get_lang_key(state)
    label = style.get(lang_key, style["fr"])
    base = style["color"]

    # Soft pulse only for strict+alert to avoid visual noise in regular operation.
    is_strict_alert = preset == "strict" and has_alert
    if is_strict_alert:
        pulse = (math.sin(time.time() * 5.0) + 1.0) / 2.0
        boost = int(28 * pulse)
        badge_color = (
            min(255, base[0] + boost),
            max(0, base[1] - boost // 2),
            max(0, base[2] - boost // 2),
        )
    else:
        badge_color = base

    badge_rect = pygame.Rect(rect.x + 14, rect.y + 12, 300, 28)
    pygame.draw.rect(surface, badge_color, badge_rect, border_radius=10)
    text = font_small.render(label, True, (8, 12, 20))
    surface.blit(text, (badge_rect.x + 10, badge_rect.y + 6))

    # Show explicit alert state only during strict-alert pulse windows.
    if is_strict_alert and pulse > 0.45:
        alert_label = GUARDIAN_ALERT_ACTIVE_LABEL.get(lang_key, GUARDIAN_ALERT_ACTIVE_LABEL["fr"])
        tag_rect = pygame.Rect(badge_rect.right + 8, badge_rect.y, 160, 28)
        pygame.draw.rect(surface, (255, 70, 70), tag_rect, border_radius=10)
        tag_text = font_small.render(alert_label, True, (12, 10, 10))
        surface.blit(tag_text, (tag_rect.x + 10, tag_rect.y + 6))


def draw_guardian_panel_alert_overlay(surface, rect, preset, has_alert):
    if not (preset == "strict" and has_alert):
        return
    pulse = (math.sin(time.time() * 5.0) + 1.0) / 2.0
    width = 2 + int(3 * pulse)
    glow = int(80 + 120 * pulse)
    color = (255, glow // 2, glow // 3)
    pygame.draw.rect(surface, color, rect, width=width, border_radius=16)


def main():
    pygame.init()
    screen = pygame.display.set_mode((1280, 720), pygame.RESIZABLE)
    pygame.display.set_caption("Keurgui - Second Screen")
    clock = pygame.time.Clock()

    font_title = pygame.font.SysFont("Segoe UI", 28, bold=True)
    font_h2 = pygame.font.SysFont("Segoe UI", 22, bold=True)
    font_text = pygame.font.SysFont("Consolas", 20)
    font_small = pygame.font.SysFont("Consolas", 16)

    default_state = os.path.join(
        os.path.dirname(__file__), "..", "faceApp", "data", "keurgui_state.json"
    )
    state_path = os.environ.get("KEURGUI_STATE_PATH", default_state)
    screenshot_path = os.environ.get("KEURGUI_SCREENSHOT_PATH", "").strip()
    screenshot_done = False

    last_reload = 0.0
    state = load_state(state_path)
    quickstart_hints = {
        1: "Step 1: Choisir langue (1=fr, 2=en, 3=wo)",
        2: "Step 2: Choisir profil (P) puis ajouter un membre (A)",
        3: "Step 3: Ajouter une routine (R)",
        4: "Step 4: Valider le foyer (Enter)",
    }

    running = True
    while running:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
            elif event.type == pygame.KEYDOWN:
                if event.key == pygame.K_ESCAPE:
                    running = False
                elif event.key == pygame.K_1:
                    mark_interaction(state)
                    state["langue"] = "fr"
                    state["quickstart"]["last_action"] = "langue=fr"
                    save_state(state_path, state)
                    maybe_emit_voice(state, "lang_fr")
                elif event.key == pygame.K_2:
                    mark_interaction(state)
                    state["langue"] = "en"
                    state["quickstart"]["last_action"] = "langue=en"
                    save_state(state_path, state)
                    maybe_emit_voice(state, "lang_en")
                elif event.key == pygame.K_3:
                    mark_interaction(state)
                    state["langue"] = "wo"
                    state["quickstart"]["last_action"] = "langue=wo"
                    save_state(state_path, state)
                    maybe_emit_voice(state, "lang_fr")
                elif event.key == pygame.K_p:
                    mark_interaction(state)
                    cycle_profile(state)
                    save_state(state_path, state)
                    maybe_emit_voice(state, "profile_changed", profile=state["onboarding_profile"])
                elif event.key == pygame.K_v:
                    mark_interaction(state)
                    state["voice_guide_enabled"] = not bool(state.get("voice_guide_enabled", False))
                    state["quickstart"]["last_action"] = f"voice_guide={state['voice_guide_enabled']}"
                    save_state(state_path, state)
                    if state.get("voice_guide_enabled"):
                        maybe_emit_voice(state, "voice_on")
                    else:
                        emit_voice_prompt(state, voice_text(state, "voice_off"))
                elif event.key == pygame.K_g:
                    mark_interaction(state)
                    state["guardian_mode_enabled"] = not bool(state.get("guardian_mode_enabled", False))
                    state["quickstart"]["last_action"] = f"guardian={state['guardian_mode_enabled']}"
                    save_state(state_path, state)
                    if state.get("guardian_mode_enabled"):
                        maybe_emit_voice(state, "guardian_on")
                    else:
                        maybe_emit_voice(state, "guardian_off")
                elif event.key == pygame.K_t:
                    mark_interaction(state)
                    preset = cycle_guardian_preset_for_profile(state)
                    state["quickstart"]["last_action"] = f"guardian_preset={preset}"
                    save_state(state_path, state)
                    maybe_emit_voice(state, "guardian_preset_applied", preset=preset)
                elif event.key == pygame.K_n:
                    mark_interaction(state)
                    modes = ["calme", "normal", "urgence"]
                    current = state.get("mode_foyer", "normal")
                    idx = modes.index(current) if current in modes else 1
                    state["mode_foyer"] = modes[(idx + 1) % len(modes)]
                    state["quickstart"]["last_action"] = f"mode={state['mode_foyer']}"
                    save_state(state_path, state)
                    maybe_emit_voice(state, "mode_changed", mode=state["mode_foyer"])
                elif event.key == pygame.K_i:
                    mark_interaction(state)
                    updated = adjust_profile_threshold(state, "idle_threshold_minutes", 5, 5, 180)
                    state["quickstart"]["last_action"] = f"guardian_idle={updated}"
                    save_state(state_path, state)
                    maybe_emit_voice(state, "threshold_idle_updated", minutes=updated)
                elif event.key == pygame.K_k:
                    mark_interaction(state)
                    updated = adjust_profile_threshold(state, "idle_threshold_minutes", -5, 5, 180)
                    state["quickstart"]["last_action"] = f"guardian_idle={updated}"
                    save_state(state_path, state)
                    maybe_emit_voice(state, "threshold_idle_updated", minutes=updated)
                elif event.key == pygame.K_o:
                    mark_interaction(state)
                    updated = adjust_profile_threshold(state, "reminder_window_minutes", 5, 5, 180)
                    state["quickstart"]["last_action"] = f"guardian_reminder={updated}"
                    save_state(state_path, state)
                    maybe_emit_voice(state, "threshold_reminder_updated", minutes=updated)
                elif event.key == pygame.K_l:
                    mark_interaction(state)
                    updated = adjust_profile_threshold(state, "reminder_window_minutes", -5, 5, 180)
                    state["quickstart"]["last_action"] = f"guardian_reminder={updated}"
                    save_state(state_path, state)
                    maybe_emit_voice(state, "threshold_reminder_updated", minutes=updated)
                elif event.key == pygame.K_a:
                    mark_interaction(state)
                    members = state.get("membres", [])
                    new_id = f"m{len(members) + 1}"
                    members.append({"id": new_id, "prenom": f"Membre{len(members)+1}", "role": "member"})
                    state["membres"] = members
                    state["quickstart"]["last_action"] = "member_added"
                    save_state(state_path, state)
                    maybe_emit_voice(state, "member_added")
                elif event.key == pygame.K_r:
                    mark_interaction(state)
                    routines = state.get("routines", [])
                    new_id = f"r{len(routines) + 1}"
                    routines.append({"id": new_id, "nom": f"Routine{len(routines)+1}", "heure": "20:00", "actif": True})
                    state["routines"] = routines
                    state["quickstart"]["last_action"] = "routine_added"
                    save_state(state_path, state)
                    maybe_emit_voice(state, "routine_added")
                elif event.key in (pygame.K_RETURN, pygame.K_SPACE):
                    mark_interaction(state)
                    quickstart_next(state)
                    save_state(state_path, state)
                    if state.get("first_life_completed"):
                        maybe_emit_voice(state, "quickstart_done")
                    else:
                        maybe_emit_voice(state, "quickstart_next")

        now = time.time()
        if now - last_reload > 1.0:
            state = load_state(state_path)
            last_reload = now

            # --- CHIRURGIE MENTALE (AI TELEKINESIS) ---
            # L'IA (Diop-Creator) peut injecter un fichier ai_morph.json pour redessiner l'interface
            try:
                ai_morph_path = os.path.join(os.path.dirname(state_path), "ai_morph.json")
                if os.path.exists(ai_morph_path):
                    with open(ai_morph_path, "r", encoding="utf-8") as f:
                        morph = json.load(f)
                    if "bg_color" in morph:
                        global BG
                        BG = tuple(morph["bg_color"])
                    if "accent_color" in morph:
                        global ACCENT
                        ACCENT = tuple(morph["accent_color"])
                    if "custom_hologram" in morph:
                        # L'IA injecte un texte holographique dans la liste "Futures"
                        if "futures" not in state:
                            state["futures"] = []
                        # On évite les doublons
                        hologram = f"👁️‍🗨️ [DIOP-CREATOR]: {morph['custom_hologram']}"
                        if hologram not in state["futures"]:
                            state["futures"].insert(0, hologram)
            except Exception:
                pass

        w, h = screen.get_size()
        screen.fill(BG)

        title = font_title.render("KEURGUI - MAISON VIVANTE", True, ACCENT)
        subtitle = font_small.render("Premiere app familiale de vie OO", True, MUTED)
        screen.blit(title, (30, 20))
        screen.blit(subtitle, (32, 58))

        foyer = state.get("nom_foyer", "Keurgui Maison")
        mode = state.get("mode_foyer", "normal")
        members = state.get("membres", [])
        routines = state.get("routines", [])
        futures = state.get("futures", [])
        lang = state.get("langue", "fr")
        profile = state.get("onboarding_profile", "general")
        voice_enabled = bool(state.get("voice_guide_enabled", False))
        guardian_enabled = bool(state.get("guardian_mode_enabled", False))
        idle_threshold, reminder_window = guardian_thresholds_for_profile(state)
        active_preset = guardian_preset_for_profile(state)
        guardian_lines = []
        guardian_has_alert = False
        guardian_lines.append(f"- Profile preset: {active_preset}")
        guardian_lines.append(f"- Profile thresholds: idle={idle_threshold}m, reminder={reminder_window}m")
        if guardian_enabled:
            alerts = guardian_alerts(state)
            guardian_has_alert = bool(alerts)
            guardian_lines.append("- Active")
            if alerts:
                for alert in alerts:
                    guardian_lines.append(f"- ALERT: {alert}")
                alert_signature = "|".join(alerts)
                if state.get("guardian_last_alert", "") != alert_signature:
                    state["guardian_last_alert"] = alert_signature
                    state["quickstart"]["last_action"] = f"guardian_alert={alerts[0]}"
                    save_state(state_path, state)
                    maybe_emit_voice(state, "guardian_alert", alert=alerts[0])
            else:
                guardian_lines.append("- No alert")
                if state.get("guardian_last_alert", ""):
                    state["guardian_last_alert"] = ""
                    save_state(state_path, state)
        else:
            guardian_lines = ["- Inactive"]
        qs = state.get("quickstart", {})
        qs_step = int(qs.get("step", 1))
        qs_total = int(qs.get("steps_total", 4))
        qs_done = bool(qs.get("completed", False))
        qs_last = qs.get("last_action", "")

        header = font_h2.render(
            f"Foyer: {foyer} | Mode: {mode} | Langue: {lang} | Profil: {profile}",
            True,
            TEXT,
        )
        screen.blit(header, (30, 95))

        left = pygame.Rect(30, 140, (w // 2) - 45, h - 190)
        right_top = pygame.Rect((w // 2) + 15, 140, (w // 2) - 45, (h - 210) // 2)
        right_bottom = pygame.Rect((w // 2) + 15, right_top.bottom + 20, (w // 2) - 45, (h - 210) // 2)

        member_lines = [f"- {m.get('prenom', 'N/A')} ({m.get('role', 'member')})" for m in members]
        if not member_lines:
            member_lines = ["- Aucun membre configure"]

        routine_lines = [
            f"- {r.get('heure', '--:--')} | {r.get('nom', 'Routine')} | actif={r.get('actif', False)}"
            for r in routines
        ]
        if not routine_lines:
            routine_lines = ["- Aucune routine configuree"]

        if qs_done:
            quick_lines = [
                "- Quickstart complete",
                "- First life state validated",
                "- Ready for daily family mode",
            ]
        else:
            hint = quickstart_hints.get(qs_step, "Step inconnu")
            quick_lines = [
                f"- Progress: {qs_step}/{qs_total}",
                f"- {hint}",
                "- Enter/Space: next step",
                "- P: cycle profile | V: toggle voice guide | G: guardian",
                "- T: cycle guardian preset (safe/balanced/strict)",
                "- N: cycle mode | A: add member | R: add routine",
                "- I/K: idle +/- 5m | O/L: reminder +/- 5m",
                f"- Voice guide: {voice_enabled}",
                f"- Guardian mode: {guardian_enabled}",
                f"- Guardian preset: {active_preset}",
                f"- Thresholds: idle={idle_threshold}m reminder={reminder_window}m",
                f"- Last interaction: {state.get('guardian_last_interaction_at', '') or 'none'}",
                f"- Last action: {qs_last if qs_last else 'none'}",
            ]

        future_lines = [f"- {f}" for f in futures] if futures else ["- Pas encore d'idee future enregistree"]

        draw_panel(screen, left, "Membres", font_h2, font_text, member_lines)
        draw_panel(screen, right_top, "Quickstart 60s", font_h2, font_text, quick_lines)
        draw_panel(screen, right_bottom, "Guardian + Futurisme", font_h2, font_text, guardian_lines + future_lines)
        draw_guardian_panel_alert_overlay(screen, right_bottom, active_preset, guardian_has_alert)
        draw_guardian_vigilance_badge_animated(
            screen,
            right_bottom,
            font_small,
            state,
            active_preset,
            guardian_has_alert,
        )

        footer = font_small.render(
            "ESC quit | 1/2/3 langue | P profil | V voice | G guardian | T preset | N mode | I/K idle | O/L reminder | A membre | R routine | Enter next",
            True,
            MUTED,
        )
        screen.blit(footer, (30, h - 30))

        pygame.display.flip()
        clock.tick(60)

        if screenshot_path and not screenshot_done:
            os.makedirs(os.path.dirname(screenshot_path), exist_ok=True)
            pygame.image.save(screen, screenshot_path)
            screenshot_done = True
            running = False

    pygame.quit()


if __name__ == "__main__":
    main()
