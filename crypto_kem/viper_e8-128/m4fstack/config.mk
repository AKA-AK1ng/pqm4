VIPER_STACK_SRCS := $(wildcard crypto_kem/viper-af67-128/m4fstack/*.c) $(wildcard crypto_kem/viper-af67-128/m4fstack/*.s) $(wildcard crypto_kem/viper-af67-128/m4fstack/*.S)

elf/crypto_kem_viper-af67-128_m4fstack_viperasmtest.elf: CPPFLAGS += -Icrypto_kem/viper-af67-128/m4fstack
elf/crypto_kem_viper-af67-128_m4fstack_viperasmtest.elf: crypto_kem/viper-af67-128/viper_asm_test.c $(VIPER_STACK_SRCS) $(LINKDEPS) $(CONFIG)
	$(compiletest)

elf/crypto_kem_viper-af67-128_m4fstack_viperprofile.elf: CPPFLAGS += -Icrypto_kem/viper-af67-128/m4fstack -DVIPER_ENABLE_PROFILE_API=1
elf/crypto_kem_viper-af67-128_m4fstack_viperprofile.elf: crypto_kem/viper-af67-128/viper_profile.c $(VIPER_STACK_SRCS) $(LINKDEPS) $(CONFIG)
	$(compiletest)

elf/crypto_kem_viper-af67-128_m4fstack_viperc_test.elf: CPPFLAGS += -Icrypto_kem/viper-af67-128/m4fstack -DVIPER_USE_ASM=0
elf/crypto_kem_viper-af67-128_m4fstack_viperc_test.elf: mupq/crypto_kem/test.c $(VIPER_STACK_SRCS) $(LINKDEPS) $(CONFIG)
	$(compiletest)

elf/crypto_kem_viper-af67-128_m4fstack_viperc_speed.elf: CPPFLAGS += -Icrypto_kem/viper-af67-128/m4fstack -DVIPER_USE_ASM=0
elf/crypto_kem_viper-af67-128_m4fstack_viperc_speed.elf: mupq/crypto_kem/speed.c $(VIPER_STACK_SRCS) $(LINKDEPS) $(CONFIG)
	$(compiletest)
