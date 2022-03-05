ISO_IMAGE = disk.iso
INITRD := vfs.tar.gz

.PHONY: all
all: $(ISO_IMAGE)

.PHONY: run
run: $(ISO_IMAGE)
	qemu-system-x86_64 -M q35 -m 2G -cdrom $(ISO_IMAGE)

ntldr:
	git clone https://github.com/limine-bootloader/limine.git --branch=v2.0-branch-binary --depth=1
	make -C limine

cock:
	ntldr/limine-install hdd.vhd --force-mbr 

.PHONY: ntoskrnl
ntoskrnl:
	$(MAKE) -C ntoskrnl

$(ISO_IMAGE): ntldr ntoskrnl
	tar czf $(INITRD) -C vfs/ ./ --format=ustar
	rm -rf iso_root
	mkdir -p iso_root
	cp ntoskrnl/ntoskrnl.elf \
		limine.cfg ntldr/limine.sys ntldr/limine-cd.bin ntldr/limine-eltorito-efi.bin iso_root/
	xorriso -as mkisofs -b limine-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table \
		--efi-boot limine-eltorito-efi.bin \
		-efi-boot-part --efi-boot-image --protective-msdos-label \
		iso_root -o $(ISO_IMAGE)
	ntldr/limine-install $(ISO_IMAGE)
	rm -rf iso_root
	$(MAKE) -C ntoskrnl clean

.PHONY: clean
clean:
	
	$(MAKE) -C ntoskrnl clean

.PHONY: distclean
distclean: clean
	rm -rf ntldr
	$(MAKE) -C ntoskrnl distclean
