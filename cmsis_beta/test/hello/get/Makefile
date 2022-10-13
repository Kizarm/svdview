all:
	wget https://www.st.com/resource/en/svd/stm32f0_svd.zip
	unzip stm32f0_svd.zip
	mv STM32F0_svd_V1.3/STM32F0x1.svd .
	patch -p2 < svd.patch
	mv STM32F0x1.svd ..
clean:
	rm -f  Release_Notes.htm stm32f0_svd.zip
	rm -rf _htmresc Release_Notes_files STM32F0_svd_V1.3
.PHONY: all clean
