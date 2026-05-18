"""
BOT-BAREMETAL — OOBridgeAgent
Interface entre le Bot-Baremetal et le LLM-Baremetal dans l'OO.

"Le Bot parle. Le LLM écoute. Et vice-versa.
 Mais le Bot n'a jamais besoin du LLM pour survivre."

Protocole :
  BOT → LLM  : événements sécurité, demandes d'analyse, alertes de survie
  LLM → BOT  : directives stratégiques optionnelles (le Bot peut refuser)

Format : JSON compatible avec le journal OO (OOJOUR.LOG style)
"""

from __future__ import annotations

import json
import time
from dataclasses import dataclass, field, asdict
from enum import IntEnum
from pathlib import Path
from typing import Optional


# ─── Types de messages ───────────────────────────────────────────────────────

class OOMsgType(IntEnum):
    BOT_EVENT     = 1   # Bot → LLM : événement de sécurité
    BOT_ALERT     = 2   # Bot → LLM : alerte critique (survie)
    BOT_QUERY     = 3   # Bot → LLM : demande d'analyse approfondie
    LLM_DIRECTIVE = 10  # LLM → Bot : directive stratégique
    LLM_ANALYSIS  = 11  # LLM → Bot : résultat d'analyse
    LLM_ACK       = 12  # LLM → Bot : acquittement


class BotDirectiveAction(IntEnum):
    DEPLOY_HONEYTRAP    = 1
    INCREASE_MONITORING = 2
    QUARANTINE_TARGET   = 3
    RELAX_ALERT         = 4
    REQUEST_GENOME_SYNC = 5
    EMERGENCY_LOCKDOWN  = 6


# ─── Structures de messages ───────────────────────────────────────────────────

@dataclass
class OOMessage:
    """Message échangé entre Bot-Baremetal et LLM-Baremetal via le journal OO."""
    oo_type:     int          # OOMsgType
    version:     int          = 1
    timestamp:   float        = field(default_factory=time.time)
    from_entity: str          = "bot-baremetal"
    to_entity:   str          = "llm-baremetal"
    msg_id:      str          = ""
    payload:     dict         = field(default_factory=dict)

    def __post_init__(self):
        if not self.msg_id:
            import hashlib
            self.msg_id = hashlib.sha256(
                f"{self.timestamp}{self.oo_type}".encode()
            ).hexdigest()[:12]

    def to_json(self) -> str:
        return json.dumps(asdict(self), indent=2, ensure_ascii=False)

    @classmethod
    def from_json(cls, raw: str) -> "OOMessage":
        d = json.loads(raw)
        return cls(**d)


@dataclass
class BotEvent:
    """Événement de sécurité du Bot destiné au LLM."""
    threat_level:   int     # 0-5
    event_type:     str     # "VIGILANCE", "COMBAT", etc.
    agent:          str     # Agent source
    description:    str
    confidence:     int     # 0-100
    action_taken:   str     # Action déjà prise par le Bot
    pattern_id:     str     = ""
    request:        str     = ""  # Optionnel : demande au LLM


@dataclass
class LLMDirective:
    """Directive du LLM vers le Bot."""
    action:     int         # BotDirectiveAction
    target:     str         = ""
    priority:   str         = "NORMAL"   # LOW, NORMAL, HIGH, CRITICAL
    context:    str         = ""
    expires_at: float       = 0.0        # 0 = pas d'expiration


# ─── OOBridgeAgent ────────────────────────────────────────────────────────────

class OOBridgeAgent:
    """
    Agent de communication entre le Bot-Baremetal et le LLM-Baremetal.

    Il écrit dans BOT_EVENTS.LOG (que le LLM lit)
    et lit depuis BOT_DIRECTIVES.LOG (que le LLM écrit).

    Règle absolue : si le LLM ne répond pas, le Bot continue seul.
    Le bridge est enrichissant, jamais critique.
    """

    def __init__(self, oo_data_path: Path):
        self.oo_data_path = oo_data_path
        self.oo_data_path.mkdir(parents=True, exist_ok=True)

        self.events_log      = oo_data_path / "BOT_EVENTS.LOG"
        self.directives_log  = oo_data_path / "BOT_DIRECTIVES.LOG"
        self.bridge_log      = oo_data_path / "OO_BRIDGE.LOG"

        self._last_directive_pos = 0
        self._sent_count         = 0
        self._received_count     = 0

        self._log(f"OOBridgeAgent initialized — data_path={oo_data_path}")

    # ── Logging interne ───────────────────────────────────────────────────────

    def _log(self, msg: str) -> None:
        ts = time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime())
        line = f"[{ts}] [OOBridge] {msg}\n"
        try:
            with open(self.bridge_log, "a", encoding="utf-8") as f:
                f.write(line)
        except OSError:
            pass  # Pas de crash si le log est inaccessible

    # ── Envoi Bot → LLM ──────────────────────────────────────────────────────

    def send_event(self, event: BotEvent) -> str:
        """
        Envoie un événement de sécurité vers le LLM.
        Retourne le msg_id pour traçabilité.
        """
        msg = OOMessage(
            oo_type=int(OOMsgType.BOT_EVENT),
            from_entity="bot-baremetal",
            to_entity="llm-baremetal",
            payload=asdict(event),
        )
        self._write_to_log(self.events_log, msg)
        self._sent_count += 1
        self._log(f"EVENT sent msg_id={msg.msg_id} threat={event.threat_level} "
                  f"agent={event.agent}")
        return msg.msg_id

    def send_alert(self, threat_level: int, description: str,
                   action_taken: str = "") -> str:
        """
        Alerte critique — utilisé en mode SURVIVAL ou CONFINEMENT.
        """
        event = BotEvent(
            threat_level=threat_level,
            event_type="SURVIVAL_ALERT" if threat_level >= 4 else "ALERT",
            agent="swarm_mind",
            description=description,
            confidence=99,
            action_taken=action_taken,
            request="ACKNOWLEDGE",
        )
        msg = OOMessage(
            oo_type=int(OOMsgType.BOT_ALERT),
            from_entity="bot-baremetal",
            to_entity="llm-baremetal",
            payload=asdict(event),
        )
        self._write_to_log(self.events_log, msg)
        self._sent_count += 1
        self._log(f"ALERT sent msg_id={msg.msg_id} level={threat_level}")
        return msg.msg_id

    def send_query(self, question: str, context: dict) -> str:
        """
        Demande une analyse approfondie au LLM pour un pattern complexe.
        Le Bot continue à fonctionner sans attendre la réponse.
        """
        msg = OOMessage(
            oo_type=int(OOMsgType.BOT_QUERY),
            from_entity="bot-baremetal",
            to_entity="llm-baremetal",
            payload={"question": question, "context": context},
        )
        self._write_to_log(self.events_log, msg)
        self._sent_count += 1
        self._log(f"QUERY sent msg_id={msg.msg_id} question={question[:60]}")
        return msg.msg_id

    # ── Réception LLM → Bot ──────────────────────────────────────────────────

    def poll_directives(self) -> list[LLMDirective]:
        """
        Lit les nouvelles directives du LLM depuis BOT_DIRECTIVES.LOG.
        Appel non bloquant — retourne [] si rien de nouveau.
        Le Bot évalue chaque directive avant de l'appliquer.
        """
        if not self.directives_log.exists():
            return []

        directives = []
        try:
            with open(self.directives_log, "r", encoding="utf-8") as f:
                f.seek(self._last_directive_pos)
                for line in f:
                    line = line.strip()
                    if not line:
                        continue
                    try:
                        msg = OOMessage.from_json(line)
                        if msg.oo_type in (int(OOMsgType.LLM_DIRECTIVE),):
                            directive = LLMDirective(**msg.payload)
                            # Vérifier expiration
                            if directive.expires_at == 0 or \
                               directive.expires_at > time.time():
                                directives.append(directive)
                                self._received_count += 1
                    except (json.JSONDecodeError, TypeError):
                        pass
                self._last_directive_pos = f.tell()
        except OSError:
            pass

        if directives:
            self._log(f"Received {len(directives)} directive(s) from LLM")
        return directives

    def evaluate_directive(self, directive: LLMDirective) -> bool:
        """
        Évalue si une directive du LLM est sûre et applicable.
        Le Bot peut refuser une directive si elle viole ses prime_directives.

        Retourne True si la directive est acceptée, False si refusée.
        """
        action = directive.action

        # Refus catégoriques (prime_directives)
        if action == int(BotDirectiveAction.EMERGENCY_LOCKDOWN):
            if directive.priority != "CRITICAL":
                self._log(f"DIRECTIVE REFUSED: EMERGENCY_LOCKDOWN requires CRITICAL priority")
                return False

        # Toujours accepté
        safe_actions = {
            int(BotDirectiveAction.DEPLOY_HONEYTRAP),
            int(BotDirectiveAction.INCREASE_MONITORING),
            int(BotDirectiveAction.REQUEST_GENOME_SYNC),
        }
        if action in safe_actions:
            self._log(f"DIRECTIVE ACCEPTED: action={action} target={directive.target}")
            return True

        # Accepté si priorité >= NORMAL
        if directive.priority in ("HIGH", "CRITICAL"):
            self._log(f"DIRECTIVE ACCEPTED (HIGH): action={action}")
            return True

        self._log(f"DIRECTIVE DEFERRED: action={action} priority={directive.priority}")
        return False

    # ── Écriture dans les logs OO ─────────────────────────────────────────────

    def _write_to_log(self, log_path: Path, msg: OOMessage) -> None:
        """Écrit un message JSON sur une seule ligne dans le log OO."""
        try:
            line = json.dumps(asdict(msg), ensure_ascii=False)
            with open(log_path, "a", encoding="utf-8") as f:
                f.write(line + "\n")
        except OSError as e:
            self._log(f"ERROR writing to {log_path}: {e}")

    # ── Statistiques ─────────────────────────────────────────────────────────

    def status_report(self) -> dict:
        return {
            "sent_to_llm":       self._sent_count,
            "received_from_llm": self._received_count,
            "events_log":        str(self.events_log),
            "directives_log":    str(self.directives_log),
        }

    def print_status(self) -> None:
        r = self.status_report()
        print("\n=== OOBridgeAgent Status ===")
        for k, v in r.items():
            print(f"  {k:25s}: {v}")
        print("============================\n")


# ─── Simulation ───────────────────────────────────────────────────────────────

if __name__ == "__main__":
    import tempfile

    with tempfile.TemporaryDirectory() as tmp:
        bridge = OOBridgeAgent(Path(tmp))

        # Simuler envoi d'un événement de combat
        mid = bridge.send_event(BotEvent(
            threat_level=3,
            event_type="COMBAT",
            agent="mem_watch",
            description="Process injection confirmed in pid 4821",
            confidence=91,
            action_taken="quarantine",
            pattern_id="abc123",
            request="ANALYZE_PATTERN",
        ))
        print(f"Event sent: {mid}")

        # Simuler une alerte critique
        mid2 = bridge.send_alert(
            threat_level=4,
            description="Bot agent swarm_mind is being targeted",
            action_taken="camouflage+regen"
        )
        print(f"Alert sent: {mid2}")

        bridge.print_status()

        # Vérifier le log écrit
        log_path = Path(tmp) / "BOT_EVENTS.LOG"
        if log_path.exists():
            lines = log_path.read_text(encoding="utf-8").strip().split("\n")
            print(f"\nBOT_EVENTS.LOG: {len(lines)} entries")
