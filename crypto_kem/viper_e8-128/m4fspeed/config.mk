VIPER_SPEED_SRCS := $(wildcard crypto_kem/viper-af67-128/m4fspeed/*.c) $(wildcard crypto_kem/viper-af67-128/m4fspeed/*.s) $(wildcard crypto_kem/viper-af67-128/m4fspeed/*.S)

elf/crypto_kem_viper-af67-128_m4fspeed_viperasmtest.elf: CPPFLAGS += -Icrypto_kem/viper-af67-128/m4fspeed
elf/crypto_kem_viper-af67-128_m4fspeed_viperasmtest.elf: crypto_kem/viper-af67-128/viper_asm_test.c $(VIPER_SPEED_SRCS) $(LINKDEPS) $(CONFIG)
	$(compiletest)

elf/crypto_kem_viper-af67-128_m4fspeed_viperprofile.elf: CPPFLAGS += -Icrypto_kem/viper-af67-128/m4fspeed -DVIPER_ENABLE_PROFILE_API=1
elf/crypto_kem_viper-af67-128_m4fspeed_viperprofile.elf: crypto_kem/viper-af67-128/viper_profile.c $(VIPER_SPEED_SRCS) $(LINKDEPS) $(CONFIG)
	$(compiletest)

elf/crypto_kem_viper-af67-128_m4fspeed_viperc_test.elf: CPPFLAGS += -Icrypto_kem/viper-af67-128/m4fspeed -DVIPER_USE_ASM=0
elf/crypto_kem_viper-af67-128_m4fspeed_viperc_test.elf: mupq/crypto_kem/test.c $(VIPER_SPEED_SRCS) $(LINKDEPS) $(CONFIG)
	$(compiletest)

elf/crypto_kem_viper-af67-128_m4fspeed_viperc_speed.elf: CPPFLAGS += -Icrypto_kem/viper-af67-128/m4fspeed -DVIPER_USE_ASM=0
elf/crypto_kem_viper-af67-128_m4fspeed_viperc_speed.elf: mupq/crypto_kem/speed.c $(VIPER_SPEED_SRCS) $(LINKDEPS) $(CONFIG)
	$(compiletest)
