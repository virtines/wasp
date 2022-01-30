CMAKE_ROOT=$(shell pwd)
BUILD:=build/
VENV:=venv/


all: wasp virtine_bins build/fib.bin libc build/echo_server.bin


wasp:
	@mkdir -p $(BUILD)
	@cd $(BUILD); cmake -DCMAKE_BUILD_TYPE=Debug $(CMAKE_ROOT)
	make --no-print-directory -C $(BUILD) -j $(shell nproc)
	@cp $(BUILD)/compile_commands.json .

build/%.virtine: test/virtines/%.asm
	mkdir -p build
	nasm -fbin -o $@ $<

virtine_bins: build/fib.virtine build/fib16.virtine build/fib32.virtine build/fib64.virtine build/boottime.virtine




clean:
	@rm -rf $(BUILD)

install:
	@mkdir -p /usr/local/lib/virtine
	@mkdir -p /usr/local/include/wasp
	@install -m 755 build/libwasp.so /usr/local/lib/libwasp.so
	@install -m 755 include/virtine.h /usr/local/include/virtine.h
	@install -m 755 -D include/wasp/* -t /usr/local/include/wasp/
	@install -m 755 build/VirtinePass.so /usr/local/lib/virtine/VirtinePass.so
	@install -m 755 build/libc.a /usr/local/lib/virtine/virtine_libc.a
	@install -m 755 build/libm.a /usr/local/lib/virtine/virtine_libm.a
	@install -m 755 pass/rt/boot64.asm /usr/local/lib/virtine/boot64.asm
	@install -m 755 pass/rt/link64.ld /usr/local/lib/virtine/link64.ld
	@install -m 755 pass/vcc /usr/local/bin/vcc




# build the javascript engine test
build/jsv/%.c.o: test/js/rt/%.c
	@mkdir -p $(dir $@)
	@echo " CC " $<
	@gcc -O3 -nostdlib -c -o $@ $^

build/ex_js_no_virtine: test/js/novirtine.cpp
	@clang++ -O3 -lm -Iinclude/ -o $@ $^

build/ex_js_no_virtine_nt: test/js/novirtine-nt.cpp
	@clang++ -O3 -lm -Iinclude/ -o $@ $^



libc: build/libc.a
build/libc.a:
	scripts/build_newlib.sh

# build jsinterp.bin from example/js/rt
test-js: build/libc.a test/jsv/virtine.c.o test/jsv/duktape.c.o
	@echo " LD " $^
	@nasm -felf64 example/js/rt/boot.asm -o build/jsv/boot.o
	@ld -T example/js/rt/rt.ld -o build/jsinterp.o build/jsv/boot.o $^ build/libc.a build/libc.a
	@objcopy -O binary build/jsinterp.o build/jsinterp.bin


build/fib.bin: test/fib/boot.asm test/fib/virtine.c
	@mkdir -p build/fib
	@nasm -felf64 test/fib/boot.asm -o build/fib/boot.o
	@gcc -O3 -nostdlib -c -o build/fib/virtine.o test/fib/virtine.c
	@ld -T test/fib/rt.ld -o build/fib.elf build/fib/boot.o build/fib/virtine.o
	@objcopy -O binary build/fib.elf build/fib.bin


js: default test-js build/ex_js_no_virtine build/ex_js_no_virtine_nt


build/echo_server.bin:
	nasm -felf32 -o build/echo_boot.o test/echo_server/boot.asm
	gcc -fno-stack-protector -fno-pie -m32 -O3 -fno-common -ffreestanding -nostdinc -nostdlib -c -o build/echo_server_main.o test/echo_server/main.c
	ld -melf_i386 -T test/echo_server/kernel.ld -o build/echo_server.elf build/echo_boot.o build/echo_server_main.o
	objcopy -O binary build/echo_server.elf build/echo_server.bin


DATADIR?=data
ALLPLOTS:=fig3.pdf fig8.pdf fig11.pdf fig12.pdf

venv:
	@python3 -m venv $(VENV)
	@$(VENV)/bin/pip install -r plotgen/requirements.txt


data/fig3/fib16.csv:
	build/test/run build/fib16.virtine > $@
data/fig3/fib32.csv:
	build/test/run build/fib32.virtine > $@
data/fig3/fib64.csv:
	build/test/run build/fib64.virtine > $@
data/fig3:
	@mkdir -p $@
fig3_data: data/fig3 data/fig3/fib16.csv data/fig3/fib32.csv data/fig3/fib64.csv
fig3.pdf: fig3_data venv
	$(VENV)/bin/python plotgen/fig3-mode-latency.py data/fig3/ $@
fig3_gold.pdf: venv
	$(VENV)/bin/python plotgen/fig3-mode-latency.py data_golden/fig3/ $@



data/fig4/echo-server.csv:
	build/test/echo_server > $@
data/fig4:
	@mkdir -p $@
fig4_data: data/fig4 data/fig4/echo-server.csv
fig4.pdf: fig4_data venv
	$(VENV)/bin/python plotgen/fig4-boot-milestones.py data/fig4/ $@
fig4_gold.pdf: venv
	$(VENV)/bin/python plotgen/fig4-boot-milestones.py data_golden/fig4/ $@



data/fig8/linux_thread.csv:
	build/test/pthread_overhead > $@

data/fig8/linux_process.csv:
	build/test/process_overhead > $@

data/fig8/wasp_create.csv:
	build/test/create > $@

data/fig8/wasp_create_cache.csv:
	build/test/create_cache > $@

data/fig8/wasp_create_cache_async.csv:
	build/test/create_cache_async > $@

data/fig8/wasp_vmrun.csv:
	build/test/vmrun > $@

data/fig8:
	@mkdir -p $@
fig8_data: data/fig8 data/fig8/linux_thread.csv data/fig8/linux_process.csv data/fig8/wasp_create.csv data/fig8/wasp_create_cache.csv data/fig8/wasp_create_cache_async.csv data/fig8/wasp_vmrun.csv
fig8.pdf: fig8_data venv
	$(VENV)/bin/python plotgen/fig8-wasp-latency.py data/fig8/ $@
fig8_gold.pdf: venv
	$(VENV)/bin/python plotgen/fig8-wasp-latency.py data_golden/fig8/ $@









data/fig11/baseline_%.csv:
	GET_BASELINE=yes build/test/fib $(patsubst data/fig11/baseline_%.csv,%, $@) > $@
data/fig11/virtine_%.csv:
	WASP_NO_SNAPSHOT=yes build/test/fib $(patsubst data/fig11/virtine_%.csv,%, $@) > $@
data/fig11/virtine+snapshot_%.csv:
	build/test/fib $(patsubst data/fig11/virtine+snapshot_%.csv,%, $@) > $@

FIG11_DATA_FILES = data/fig11/baseline_0.csv \
                   data/fig11/baseline_5.csv \
                   data/fig11/baseline_10.csv \
                   data/fig11/baseline_15.csv \
                   data/fig11/baseline_20.csv \
                   data/fig11/baseline_25.csv \
                   data/fig11/baseline_30.csv \
                   data/fig11/virtine_0.csv \
                   data/fig11/virtine_5.csv \
                   data/fig11/virtine_10.csv \
                   data/fig11/virtine_15.csv \
                   data/fig11/virtine_20.csv \
                   data/fig11/virtine_25.csv \
                   data/fig11/virtine_30.csv \
                   data/fig11/virtine+snapshot_0.csv \
                   data/fig11/virtine+snapshot_5.csv \
                   data/fig11/virtine+snapshot_10.csv \
                   data/fig11/virtine+snapshot_15.csv \
                   data/fig11/virtine+snapshot_20.csv \
                   data/fig11/virtine+snapshot_25.csv \
                   data/fig11/virtine+snapshot_30.csv 
data/fig11:
	@mkdir -p $@
fig11_data: data/fig11 $(FIG11_DATA_FILES)
fig11.pdf: fig11_data venv
	@$(VENV)/bin/python plotgen/fig11-fib-latency.py data/fig11 $@
fig11_gold.pdf: venv
	@$(VENV)/bin/python plotgen/fig11-fib-latency.py data_golden/fig11 $@







data/fig12/image_size.csv:
	build/test/image_size > $@
data/fig12:
	@mkdir -p $@
fig12_data: data/fig12 data/fig12/image_size.csv
fig12.pdf: fig12_data venv
	$(VENV)/bin/python plotgen/fig12-image-size.py data/fig12/ $@
fig12_gold.pdf: venv
	$(VENV)/bin/python plotgen/fig12-image-size.py data_golden/fig12/ $@



data/table1.csv:
	@mkdir -p data
	build/test/boottime > data/table1.csv

table1_data: data/table1.csv


alldata: table1_data fig3_data fig8_data fig11_data fig12_data
artifacts: all alldata $(ALLPLOTS)




.PHONY: test $(ALLPLOTS) wasp
