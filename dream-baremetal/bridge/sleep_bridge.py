import ctypes
import time
from pathlib import Path
import sys

# Ajout du path pour importer DIOP (Cortex)
sys.path.append(str(Path(__file__).parent.parent.parent / "llm-baremetal"))
from diop.evolution.sleep_learning.engine import SleepLearningEngine

class DreamDaemonBridge:
    """
    Pont entre le Daemon C (dream_daemon.c) et le moteur d'apprentissage Python.
    En mode "vrai baremetal", cela se traduirait par un binaire compilé (MicroPython ou Rust).
    """
    def __init__(self, lib_path: str, memory_root: str):
        # Chargement de la librairie C partagée (le Myocarde/Dream)
        # self.c_lib = ctypes.CDLL(lib_path)
        
        self.engine = SleepLearningEngine(memory_root=Path(memory_root))
        self.is_sleeping = False

    def listen_to_bloodstream(self):
        print("[DreamBridge] 🩸 Connecté au flux sanguin. En attente de Globules Jaunes (Sommeil)...")
        
        # Simulation d'une boucle d'écoute sur le bus mémoire (united_bus)
        try:
            while True:
                # En réalité : status = self.c_lib.get_dream_status()
                # Pour le concept : on mock l'endormissement
                time.sleep(2)
                
                # Fausse réception d'un état RELAXED du Kernel
                self._trigger_sleep_cycle()
                
                # Une fois consolidé, on s'arrête (ou on attend la prochaine nuit)
                break
                
        except KeyboardInterrupt:
            print("[DreamBridge] Réveil brutal (Interrupt).")

    def _trigger_sleep_cycle(self):
        if not self.is_sleeping:
            print("\n[DreamBridge] 🟡 Globule Jaune reçu : État RELAXED. Début du rêve.")
            self.is_sleeping = True
            
            # Appel du code natif Diop
            new_memories = self.engine.run_sleep_cycle()
            
            if new_memories:
                print(f"[DreamBridge] 🧬 Mémoire ADN mise à jour : {len(new_memories)} nouveaux anticorps cognitifs.")
            
            self.is_sleeping = False
            print("[DreamBridge] 🟡 Fin du rêve. Prêt au réveil.")

if __name__ == "__main__":
    bridge = DreamDaemonBridge(lib_path="dream_daemon.so", memory_root="../../data/memory")
    bridge.listen_to_bloodstream()
