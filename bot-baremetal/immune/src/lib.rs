//! Bot-Immune Library Root
pub mod swarm_mind;
pub mod mem_watch;
pub mod fs_watch;
pub mod net_watch;
pub mod proc_watch;
pub mod quarantine;
pub mod honey_trap;
pub mod regen;
pub mod chameleon;
pub mod neutralizer;
pub mod kernel_watch;
pub mod boot_watch;

#[cfg(test)]
mod swarm_tests;

pub use swarm_mind::{SwarmMind, SwarmEvent, ThreatLevel, AgentRole};
