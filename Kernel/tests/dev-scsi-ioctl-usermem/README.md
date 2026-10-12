# scsi: validate the entire raw command buffer and select user mapping

Partial valaddr success permits a full-length transfer beyond the validated user range. The ioctl sets blk_op.is_user even though the SCSI transport maps memory using si_user.

Run `python3 Kernel/tests/dev-scsi-ioctl-usermem/check.py`. This compiles the actual driver functions with deterministic stubs and AddressSanitizer/UndefinedBehaviorSanitizer. It does not exercise physical hardware.
