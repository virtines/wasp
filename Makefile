CMAKE_ROOT=$(shell pwd)
BUILD:=build/



default:
	@mkdir -p $(BUILD)
	@cd $(BUILD); cmake $(CMAKE_ROOT)
	make --no-print-directory -C $(BUILD) -j $(shell nproc)
	@cp $(BUILD)/compile_commands.json .

build/%.virtine: bench/virtines/%.asm
	mkdir -p build
	nasm -fbin -o $@ $<

virtine_bins: build/fib16.virtine build/fib32.virtine build/fib64.virtine




clean:
	@rm -rf build 

install:
	@make -C $(BUILD) --no-print-directory install




# build the javascript engine test
build/jsv/%.c.o: bench/js/rt/%.c
	@mkdir -p $(dir $@)
	@echo " CC " $<
	@gcc -O3 -nostdlib -c -o $@ $^

build/ex_js_no_virtine: bench/js/novirtine.cpp
	@clang++ -O3 -lm -Iinclude/ -o $@ $^

build/ex_js_no_virtine_nt: bench/js/novirtine-nt.cpp
	@clang++ -O3 -lm -Iinclude/ -o $@ $^

# build jsinterp.bin from example/js/rt
test-js: bench/jsv/virtine.c.o bench/jsv/duktape.c.o
	@echo " LD " $^
	@nasm -felf64 example/js/rt/boot.asm -o build/jsv/boot.o
	@ld -T example/js/rt/rt.ld -o build/jsinterp.o build/jsv/boot.o $^ /usr/local/lib/virtine/virtine_libc.a /usr/local/lib/virtine/virtine_libm.a
	@objcopy -O binary build/jsinterp.o build/jsinterp.bin


build/fib.bin: example/fib/boot.asm example/fib/virtine.c
	@mkdir -p build/fib
	@nasm -felf64 example/fib/boot.asm -o build/fib/boot.o
	@gcc -O3 -nostdlib -c -o build/fib/virtine.o example/fib/virtine.c
	@ld -T example/fib/rt.ld -o build/fib.elf build/fib/boot.o build/fib/virtine.o
	@objcopy -O binary build/fib.elf build/fib.bin


js: default test-js build/ex_js_no_virtine build/ex_js_no_virtine_nt



DATADIR?=data
ALLPLOTS:=fig8.pdf fig12.pdf

allplots: $(ALLPLOTS)



data/fig3/fib16.csv:
	build/bench/bench_run build/fib16.virtine > $@
data/fig3/fib32.csv:
	build/bench/bench_run build/fib32.virtine > $@
data/fig3/fib64.csv:
	build/bench/bench_run build/fib64.virtine > $@
data/fig3:
	mkdir -p $@
fig3_data: data/fig3 data/fig3/fib16.csv data/fig3/fib32.csv data/fig3/fib64.csv
fig3.pdf: fig3_data
	plotgen/fig3-mode-latency.py data/fig3/ $@
fig3_gold.pdf:
	plotgen/fig3-mode-latency.py data_golden/fig3/ $@


data/fig8/linux_thread.csv:
	build/bench/bench_pthread > $@
data/fig8/linux_process.csv:
	build/bench/bench_process > $@
data/fig8/wasp_create.csv:
	build/bench/bench_create > $@
data/fig8/wasp_create_cache.csv:
	build/bench/bench_create_cache > $@
data/fig8/wasp_create_cache_async.csv:
	build/bench/bench_create_cache_async > $@
data/fig8/wasp_vmrun.csv:
	build/bench/bench_vmrun > $@
data/fig8:
	mkdir -p $@
fig8_data: data/fig8 data/fig8/linux_thread.csv data/fig8/linux_process.csv data/fig8/wasp_create.csv data/fig8/wasp_create_cache.csv data/fig8/wasp_create_cache_async.csv data/fig8/wasp_vmrun.csv
fig8.pdf: fig8_data
	plotgen/fig8-wasp-latency.py data/fig8/ $@
fig8_gold.pdf:
	plotgen/fig8-wasp-latency.py data_golden/fig8/ $@



data/fig12/image_size.csv:
	build/bench/bench_image_size > $@
data/fig12:
	mkdir -p $@
fig12_data: data/fig12 data/fig12/image_size.csv
fig12.pdf: fig12_data
	plotgen/fig12-image-size.py data/fig12/ $@
fig12_gold.pdf:
	plotgen/fig12-image-size.py data_golden/fig12/ $@







.PHONY: bench $(ALLPLOTS)
