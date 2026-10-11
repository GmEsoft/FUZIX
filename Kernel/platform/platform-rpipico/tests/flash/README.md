Run `make check` with a host C compiler supporting ASan/UBSan. The tests compile
the actual devflash.c, Dhara map and journal. Only NAND erase/program/read and
block-device registration are mocked. The NAND fixture enforces erase geometry
and one-way programming. Tests use the registered transfer/flush/trim callbacks,
reinitialize the actual driver to simulate restart, and verify overwritten/new
sectors persist without a separate flush, and trims persist after flush. The
fixture uses the actual flash-layout.py generator for its geometry. An injected
backend failure during a dirty trim checkpoint must return
EIO and preserve the previous checkpoint on restart.

Remove flash-test before changing PLATFORM_DIR to another checkout. The
unchanged e35ad7c driver fails because no flush callback is registered. This
models checkpoint durability; it does not reproduce physical flash wear,
electrical power cuts or the low-level SDK flash operations.
