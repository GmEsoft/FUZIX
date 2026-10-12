# scsi: return command errors and deselect on command-phase failure

si_docmd discards data/status errors and always returns success after selection, defeating retries and admitting failed probes. Command-phase failure returns without clearing or deselecting the controller.

Run `python3 Kernel/tests/dev-scsi-command-errors/check.py`. This compiles the actual driver functions with deterministic stubs and AddressSanitizer/UndefinedBehaviorSanitizer. It does not exercise physical hardware.
