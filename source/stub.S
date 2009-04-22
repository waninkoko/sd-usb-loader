	.rodata

	.globl bgImg
	.balign 32
bgImg:
	.incbin "../data/background"


	.globl coverImg
	.balign 32
coverImg:
	.incbin "../data/nocover"
