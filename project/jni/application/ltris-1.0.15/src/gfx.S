/***************************************************************************
                          gfx.S  -  description
                             -------------------
    begin                : Mon Mar 13 2000
    copyright            : (C) 2000 by Michael Speck
    email                :
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

.macro	CrtStack
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ebx
.endm


 
.macro DelStack
	popl	%ebx
	popl	%ebp
.endm

	.data
	.balign	16

.globl rrShft
rrShft:		.byte	0
.globl grShft
grShft:		.byte	0
.globl brShft
brShft:		.byte	0
.globl rlShft
rlShft:		.byte	0
.globl glShft
glShft:		.byte	0
.globl blShft
blShft:		.byte	0
.globl rMask
rMask:		.long	0
.globl gMask
gMask:		.long	0
.globl bMask
bMask:		.long	0
.globl bpp
bpp:		.byte	0
.globl pixelsize
pixelsize:	.byte	0
.globl clp_lx
clp_lx:		.long	0
.globl clp_ty
clp_ty:		.long	0
.globl clp_rx
clp_rx:		.long	0
.globl clp_by
clp_by:		.long	0

	.text
	.balign	16

.globl	SD_Pnt
SD_Pnt:
	CrtStack

	//x < lx
	movl	clp_lx, %eax
	cmp		%eax, 16(%ebp)
	jl		9f
	//y < ty
	movl	clp_ty, %eax
	cmp		%eax, 20(%ebp)
	jl		9f
	//x > rx
	movl	clp_rx, %eax
	cmp		%eax, 16(%ebp)
	jg		9f
	//y > ry
	subl	clp_by, %eax
	cmp		%eax, 20(%ebp)
	jg		9f

	xorw	%bx, %bx
	movb	26(%ebp), %bl	//get red part
	movb	rrShft, %cl
	shrb	%cl, %bl
	movb	rlShft, %cl
	shlw	%cl, %bx

	xorw	%ax, %ax
	movb	25(%ebp), %al	//get green part
	movb	grShft, %cl
	shrb	%cl, %al
	movb	glShft, %cl
	shlw	%cl, %ax
	orw		%ax, %bx

	xorw	%ax, %ax
	movb	24(%ebp), %al	//get blue part
	movb	brShft, %cl
	shrb	%cl, %al
	movb	blShft, %cl
 	shlw	%cl, %ax
	orw		%ax, %bx	//wordvalue in bx

	movl	20(%ebp), %eax //y
	movl	12(%ebp), %ecx //w
	mull	%ecx
	addl	16(%ebp), %eax //offset in eax

	movl	8(%ebp), %ecx
	movw	%bx, (%ecx,%eax,2)

9:
	DelStack
	ret


.globl	SD_Box
SD_Box:
	CrtStack
	pushl	%edi

	//x < lx ?
	movl	clp_lx, %eax
	cmpl	%eax, 16(%ebp)
	jge     0f
	movl	16(%ebp), %ebx
	subl	%ebx, %eax
	subl	%eax, 24(%ebp)
	movl	clp_lx, %eax
	movl	%eax, 16(%ebp)
0:
	//y < ty ?
	movl	clp_ty, %eax
	cmp		%eax, 20(%ebp)
	jge     1f
	movl    20(%ebp), %ebx
	subl	%ebx, %eax
	subl	%eax, 28(%ebp)
	movl	clp_ty, %eax
	movl	%eax, 20(%ebp)
1:
	//x > rx
    movl	clp_rx, %eax
    movl	16(%ebp), %ebx
    addl	24(%ebp), %ebx
    subl	$1, %ebx
    cmp		%eax, %ebx
    jle		2f
    movl	clp_rx, %ebx
    movl	16(%ebp), %eax
    subl	%eax, %ebx
    addl	$1, %ebx
    movl	%ebx, 24(%ebp)
2:
	//y > ty
    movl	clp_by, %eax
    movl	20(%ebp), %ebx
    subl	$1, %ebx
    addl	28(%ebp), %ebx
    cmp		%eax, %ebx
    jle		3f
    movl	clp_by, %ebx
    movl	20(%ebp), %eax
    subl	%eax, %ebx
    addl	$1, %ebx
    movl	%ebx, 28(%ebp)
3:
	
	//valid w?
	movl	24(%ebp), %eax
	cmpl	$0, %eax
	jle		4f
    //valid h?
	movl	28(%ebp), %eax
	cmpl	$0, %eax
	jle		4f
	
	xorw	%bx, %bx
	movb	34(%ebp), %bl	//get red part
	movb	rrShft, %cl
	shrb	%cl, %bl
	movb	rlShft, %cl
	shlw	%cl, %bx

	xorw	%ax, %ax
	movb	33(%ebp), %al	//get green part
	movb	grShft, %cl
	shrb	%cl, %al
	movb	glShft, %cl
	shlw	%cl, %ax
	orw		%ax, %bx

	xorw	%ax, %ax
	movb	32(%ebp), %al	//get blue part
	movb	brShft, %cl
	shrb	%cl, %al
	movb	blShft, %cl
	shlw	%cl, %ax
	orw		%bx, %ax	//wordvalue in ax
	pushw	%ax;

	movl	20(%ebp), %eax	//load y_pos
	mull	12(%ebp)
	addl	16(%ebp), %eax	
	movl	%eax, %edi
	shll	$1, %edi	//offset in lpdest
	addl	8(%ebp), %edi	//add lpdest_EA

	pushl	%ds
	popl	%es

	movl	28(%ebp), %edx	//get height

	movl	12(%ebp), %ebx
	subl	24(%ebp), %ebx
	shll	$1, %ebx //edi add-on

	popw	%ax
lp:
	movl	24(%ebp), %ecx
	rep; 	stosw
	addl	%ebx, %edi
	decl	%edx
	jnz		lp

4:
	popl	%edi
	DelStack
	ret


.globl	SD_Rct
SD_Rct:
	CrtStack
	pushl	%edi

	//x < lx ?
	movl	clp_lx, %eax
	cmpl	%eax, 16(%ebp)
	jge     0f
	movl	16(%ebp), %ebx
	subl	%ebx, %eax
	subl	%eax, 24(%ebp)
	movl	clp_lx, %eax
	movl	%eax, 16(%ebp)
0:
	//y < ty ?
	movl	clp_ty, %eax
	cmp		%eax, 20(%ebp)
	jge     1f
	movl    20(%ebp), %ebx
	subl	%ebx, %eax
	subl	%eax, 28(%ebp)
	movl	clp_ty, %eax
	movl	%eax, 20(%ebp)
1:
	//x > rx
    movl	clp_rx, %eax
    movl	16(%ebp), %ebx
    addl	24(%ebp), %ebx
    subl	$1, %ebx
    cmp		%eax, %ebx
    jle		2f
    movl	clp_rx, %ebx
    movl	16(%ebp), %eax
    subl	%eax, %ebx
    addl	$1, %ebx
    movl	%ebx, 24(%ebp)
2:
	//y > ty
    movl	clp_by, %eax
    movl	20(%ebp), %ebx
    subl	$1, %ebx
    addl	28(%ebp), %ebx
    cmp		%eax, %ebx
    jle		3f
    movl	clp_by, %ebx
    movl	20(%ebp), %eax
    subl	%eax, %ebx
    addl	$1, %ebx
    movl	%ebx, 28(%ebp)
3:
	
	//valid w?
	movl	24(%ebp), %eax
	cmpl	$0, %eax
	jle		4f
    //valid h?
	movl	28(%ebp), %eax
	cmpl	$0, %eax
	jle		4f
	
	xorw	%bx, %bx
	movb	34(%ebp), %bl	//get red part
	movb	rrShft, %cl
	shrb	%cl, %bl
	movb	rlShft, %cl
	shlw	%cl, %bx

	xorw	%ax, %ax
	movb	33(%ebp), %al	//get green part
	movb	grShft, %cl
	shrb	%cl, %al
	movb	glShft, %cl
	shlw	%cl, %ax
	orw		%ax, %bx

	xorw	%ax, %ax
	movb	32(%ebp), %al	//get blue part
	movb	brShft, %cl
	shrb	%cl, %al
	movb	blShft, %cl
	shlw	%cl, %ax
	orw		%bx, %ax	//wordvalue in ax
	pushw	%ax;

	movl	20(%ebp), %eax	//load y_pos
	mull	12(%ebp)
	addl	16(%ebp), %eax	
	movl	%eax, %edi
	shll	$1, %edi	//offset in lpdest
	addl	8(%ebp), %edi	//add lpdest_EA

	pushl	%ds
	popl	%es

	movl	28(%ebp), %edx	//get height
	subl	$2, %edx

	movl	12(%ebp), %ebx
	subl	24(%ebp), %ebx
	shll	$1, %ebx //edi add-on

	popw	%ax
	movl	24(%ebp), %ecx
	rep; 	stosw
	addl	%ebx, %edi
	
	cmp		$0, %edx
	jle		9f
lp2:
	movw	%ax, (%edi)
	movl	24(%ebp), %ecx
	subl	$1, %ecx
	shll	$1, %ecx
	addl	%ecx, %edi;
	movw	%ax, (%edi)
	addl	$2, %edi;
	addl	%ebx, %edi
	decl	%edx
	jnz		lp2

9:
	movl	24(%ebp), %ecx
	rep; 	stosw

4:
	popl	%edi
	DelStack
	ret

.globl	SD_CBx
SD_CBx:
	CrtStack
	pushl	%edi

	//x < lx ?
	movl	clp_lx, %eax
	cmpl	%eax, 16(%ebp)
	jge     0f
	movl	16(%ebp), %ebx
	subl	%ebx, %eax
	subl	%eax, 24(%ebp)
	movl	clp_lx, %eax
	movl	%eax, 16(%ebp)
0:
	//y < ty ?
	movl	clp_ty, %eax
	cmp		%eax, 20(%ebp)
	jge     1f
	movl    20(%ebp), %ebx
	subl	%ebx, %eax
	subl	%eax, 28(%ebp)
	movl	clp_ty, %eax
	movl	%eax, 20(%ebp)
1:
	//x > rx
    movl	clp_rx, %eax
    movl	16(%ebp), %ebx
    addl	24(%ebp), %ebx
    subl	$1, %ebx
    cmp		%eax, %ebx
    jle		2f
    movl	clp_rx, %ebx
    movl	16(%ebp), %eax
    subl	%eax, %ebx
    addl	$1, %ebx
    movl	%ebx, 24(%ebp)
2:
	//y > ty
    movl	clp_by, %eax
    movl	20(%ebp), %ebx
    subl	$1, %ebx
    addl	28(%ebp), %ebx
    cmp		%eax, %ebx
    jle		3f
    movl	clp_by, %ebx
    movl	20(%ebp), %eax
    subl	%eax, %ebx
    addl	$1, %ebx
    movl	%ebx, 28(%ebp)
3:

	//valid w?
	movl	24(%ebp), %eax
	cmpl	$0, %eax
	jle		4f
    //valid h?
	movl	28(%ebp), %eax
	cmpl	$0, %eax
	jle		4f
	
	xorl	%edi, %edi
	xorl	%eax, %eax

	//edi destination
	pushl	%ds
	popl	%es
	movl	20(%ebp), %eax
	mull	12(%ebp)
	addl	16(%ebp), %eax
	shll	$1, %eax
	addl	8(%ebp), %eax
	movl	%eax, %edi //dest offset in edi

	movl	28(%ebp), %edx //height
outerl5:
	movl	24(%ebp), %ecx //width
	pushl	%edx
innerl5:
	pushl	%ecx

	xor		%eax, %eax
	movb	34(%ebp), %al
	movb	rrShft, %cl
	shrb	%cl, %al
	mull	36(%ebp)
	movb	rlShft, %cl
	shll	%cl, %eax
	pushl	%eax //relative red source part
	movl	(%edi), %eax
	andl	rMask, %eax
	shrl	%cl, %eax
	movl	$100, %ebx
	subl	36(%ebp), %ebx
	mull	%ebx
	shll	%cl, %eax //relative red dest part
	popl	%ebx
	addl	%ebx, %eax
	movl	$100, %ebx
	divl	%ebx
	andl	rMask, %eax
	pushl	%eax

	xor		%eax, %eax
	movb	33(%ebp), %al
	movb	grShft, %cl
	shrb	%cl, %al
	mull	36(%ebp)
	movb	glShft, %cl
	shll	%cl, %eax
	pushl	%eax //relative green source part
	movl	(%edi), %eax
	andl	gMask, %eax
	shrl	%cl, %eax
	movl	$100, %ebx
	subl	36(%ebp), %ebx
	mull	%ebx
	shll	%cl, %eax //relative green dest part
	popl	%ebx
	addl	%ebx, %eax
	movl	$100, %ebx
	divl	%ebx
	andl	gMask, %eax
	pushl	%eax

	xor		%eax, %eax
	movb	32(%ebp), %al
	movb	brShft, %cl
	shrb	%cl, %al
	mull	36(%ebp)
	movb	blShft, %cl
	shll	%cl, %eax
	pushl	%eax //relative blue source part
	movl	(%edi), %eax
	andl	bMask, %eax
	shrl	%cl, %eax
	movl	$100, %ebx
	subl	36(%ebp), %ebx
	mull	%ebx
	shll	%cl, %eax //relative blue dest part
	popl	%ebx
	addl	%ebx, %eax
	movl	$100, %ebx
	divl	%ebx
	andl	bMask, %eax

	popl	%ebx
	orl		%ebx, %eax
	popl	%ebx
	orl		%ebx, %eax

	movw	%ax, (%edi)

	addl	$2, %edi
	popl	%ecx
	decl	%ecx
	jnz		innerl5

	popl	%edx

	movl	12(%ebp), %ebx
	subl	24(%ebp), %ebx
	shll	$1, %ebx
	addl	%ebx, %edi

	decl	%edx
	jnz		outerl5
	
4:
	popl	%edi
	DelStack
	ret

.globl	SC_Opq
SC_Opq:
	CrtStack
	pushl	%edi
	pushl	%esi

	//x < lx ?
	movl	clp_lx, %eax
	cmpl	%eax, 16(%ebp)
	jge     0f
	movl	16(%ebp), %ebx
	subl	%ebx, %eax
	subl	%eax, 40(%ebp)
	addl	%eax, 32(%ebp)
	movl	clp_lx, %eax
	movl	%eax, 16(%ebp)
0:
	//y < ty ?
	movl	clp_ty, %eax
	cmp		%eax, 20(%ebp)
	jge     1f
	movl	20(%ebp), %ebx
	subl	%ebx, %eax
	subl	%eax, 44(%ebp)
	addl	%eax, 36(%ebp)
	movl	clp_ty, %eax
	movl	%eax, 20(%ebp)
1:
	//x > rx
    movl	clp_rx, %eax
    movl	16(%ebp), %ebx
    addl	40(%ebp), %ebx
    subl	$1, %ebx
    cmp		%eax, %ebx
    jle		2f
    movl	clp_rx, %ebx
    movl	16(%ebp), %eax
    subl	%eax, %ebx
    addl	$1, %ebx
    movl	%ebx, 40(%ebp)
2:
	//y > ty
    movl	clp_by, %eax
    movl	20(%ebp), %ebx
    addl	44(%ebp), %ebx
    subl	$1, %ebx
    cmp		%eax, %ebx
    jle		3f
    movl	clp_by, %ebx
    movl	20(%ebp), %eax
    subl	%eax, %ebx
    addl	$1, %ebx
    movl	%ebx, 44(%ebp)
3:
	
	//valid w?
	movl	40(%ebp), %eax
	cmpl	$0, %eax
	jle		4f
    //valid h?
	movl	44(%ebp), %eax
	cmpl	$0, %eax
	jle		4f

	xorl	%edi, %edi
	xorl	%esi, %esi
	xorl	%eax, %eax

	//esi source
	movl	36(%ebp), %eax //sy
	mull	28(%ebp) //dw
	addl	32(%ebp), %eax //sx
	shll	$1, %eax
	addl 	24(%ebp), %eax
	movl	%eax, %esi //source offset in esi

	//edi destination
	pushl	%ds
	popl	%es
	movl	20(%ebp), %eax
	mull	12(%ebp)
	addl	16(%ebp), %eax
	shll	$1, %eax
	addl	8(%ebp), %eax
	movl	%eax, %edi //dest offset in edi

	movl	12(%ebp), %eax
	subl	40(%ebp), %eax
	shll	$1, %eax //edi add-on after completed line

	movl	28(%ebp), %ebx
	subl	40(%ebp), %ebx
	shll	$1, %ebx //esi add-on after completed line

	movl	44(%ebp), %edx //height
outerl3:
	movl	40(%ebp), %ecx //line width
	rep;	movsw
	addl	%eax, %edi
	addl	%ebx, %esi
	dec	%edx
	jnz	outerl3

4:
	popl	%esi
	popl	%edi
	DelStack
	ret

.globl	SC_Clp
SC_Clp:
	CrtStack
	pushl	%edi
	pushl	%esi

	//x < lx ?
	movl	clp_lx, %eax
	cmpl	%eax, 16(%ebp)
	jge     0f
	movl	16(%ebp), %ebx
	subl	%ebx, %eax
	subl	%eax, 40(%ebp)
	addl	%eax, 32(%ebp)
	movl	clp_lx, %eax
	movl	%eax, 16(%ebp)
0:
	//y < ty ?
	movl	clp_ty, %eax
	cmp		%eax, 20(%ebp)
	jge     1f
	movl	20(%ebp), %ebx
	subl	%ebx, %eax
	subl	%eax, 44(%ebp)
	addl	%eax, 36(%ebp)
	movl	clp_ty, %eax
	movl	%eax, 20(%ebp)
1:
	//x > rx
    movl	clp_rx, %eax
    movl	16(%ebp), %ebx
    addl	40(%ebp), %ebx
    subl	$1, %ebx
    cmp		%eax, %ebx
    jle		2f
    movl	clp_rx, %ebx
    movl	16(%ebp), %eax
    subl	%eax, %ebx
    addl	$1, %ebx
    movl	%ebx, 40(%ebp)
2:
	//y > ty
    movl	clp_by, %eax
    movl	20(%ebp), %ebx
    addl	44(%ebp), %ebx
    subl	$1, %ebx
    cmp		%eax, %ebx
    jle		3f
    movl	clp_by, %ebx
    movl	20(%ebp), %eax
    subl	%eax, %ebx
    addl	$1, %ebx
    movl	%ebx, 44(%ebp)
3:
	
	//valid w?
	movl	40(%ebp), %eax
	cmpl	$0, %eax
	jle		4f
    //valid h?
	movl	44(%ebp), %eax
	cmpl	$0, %eax
	jle		4f

	xorl	%edi, %edi
	xorl	%esi, %esi
	xorl	%eax, %eax

	//esi source
	movl	36(%ebp), %eax //sy
	mull	28(%ebp) //dw
	addl	32(%ebp), %eax //sx
	shll	$1, %eax
	addl 	24(%ebp), %eax
	movl	%eax, %esi //source offset in esi

	//edi destination
	pushl	%ds
	popl	%es
	movl	20(%ebp), %eax
	mull	12(%ebp)
	addl	16(%ebp), %eax
	shll	$1, %eax
	addl	8(%ebp), %eax
	movl	%eax, %edi //dest offset in edi

	movl	12(%ebp), %eax
	subl	40(%ebp), %eax
	shll	$1, %eax //edi add-on after completed line

	movl	28(%ebp), %ebx
	subl	40(%ebp), %ebx
	shll	$1, %ebx //esi add-on after completed line

	movl	44(%ebp), %edx
outerl2:
	movl	40(%ebp), %ecx
innerl2:
	pushl	%ecx
	movw	(%esi), %cx
	cmp		$0, %cx
	jz		nopixel
	movw	(%esi), %cx
	movw	%cx, (%edi)
nopixel:
	addl	$2, %esi
	addl	$2, %edi
	popl	%ecx
	decl	%ecx
	jnz		innerl2

	addl	%eax, %edi
	addl	%ebx, %esi

	decl	%edx
	jnz		outerl2

4:
	popl	%esi
	popl	%edi
	DelStack
	ret

.globl	SC_Trp_11
SC_Trp_11:
	CrtStack
	pushl	%edi
	pushl	%esi

	//x < lx ?
	movl	clp_lx, %eax
	cmpl	%eax, 16(%ebp)
	jge     0f
	movl	16(%ebp), %ebx
	subl	%ebx, %eax
	subl	%eax, 40(%ebp)
	addl	%eax, 32(%ebp)
	movl	clp_lx, %eax
	movl	%eax, 16(%ebp)
0:
	//y < ty ?
	movl	clp_ty, %eax
	cmp		%eax, 20(%ebp)
	jge     1f
	movl	20(%ebp), %ebx
	subl	%ebx, %eax
	subl	%eax, 44(%ebp)
	addl	%eax, 36(%ebp)
	movl	clp_ty, %eax
	movl	%eax, 20(%ebp)
1:
	//x > rx
    movl	clp_rx, %eax
    movl	16(%ebp), %ebx
    addl	40(%ebp), %ebx
    subl	$1, %ebx
    cmp		%eax, %ebx
    jle		2f
    movl	clp_rx, %ebx
    movl	16(%ebp), %eax
    subl	%eax, %ebx
    addl	$1, %ebx
    movl	%ebx, 40(%ebp)
2:
	//y > ty
    movl	clp_by, %eax
    movl	20(%ebp), %ebx
    addl	44(%ebp), %ebx
    subl	$1, %ebx
    cmp		%eax, %ebx
    jle		3f
    movl	clp_by, %ebx
    movl	20(%ebp), %eax
    subl	%eax, %ebx
    addl	$1, %ebx
    movl	%ebx, 44(%ebp)
3:
	
	//valid w?
	movl	40(%ebp), %eax
	cmpl	$0, %eax
	jle		4f
    //valid h?
	movl	44(%ebp), %eax
	cmpl	$0, %eax
	jle		4f

	xorl	%edi, %edi
	xorl	%esi, %esi
	xorl	%eax, %eax

	//esi source
	movl	36(%ebp), %eax //sy
	mull	28(%ebp) //dw
	addl	32(%ebp), %eax //sx
	shll	$1, %eax
	addl 	24(%ebp), %eax
	movl	%eax, %esi //source offset in esi

	//edi destination
	pushl	%ds
	popl	%es
	movl	20(%ebp), %eax
	mull	12(%ebp)
	addl	16(%ebp), %eax
	shll	$1, %eax
	addl	8(%ebp), %eax
	movl	%eax, %edi //dest offset in edi

	movl	44(%ebp), %edx
outerl:
	movl	40(%ebp), %ecx
innerl:
	movw	(%esi), %bx
	cmpw	$0, %bx
	jz		7f

	movl	(%esi), %eax
	andl	rMask, %eax
	movl	(%edi), %ebx
	andl	rMask, %ebx
	addl	%ebx, %eax
	shrl	$1, %eax
	andl	rMask, %eax
	pushl	%eax

	movl	(%esi), %eax
	andl	gMask, %eax
	movl	(%edi), %ebx
	andl	gMask, %ebx
	addl	%ebx, %eax
	shrl	$1, %eax
	andl	gMask, %eax
	pushl	%eax

	movl	(%esi), %eax
	andl	bMask, %eax
	movl	(%edi), %ebx
	andl	bMask, %ebx
	addl	%ebx, %eax
	shrl	$1, %eax
	andl	bMask, %eax

	popl	%ebx
	orl		%ebx, %eax
	popl	%ebx
	orl		%ebx, %eax

	movw	%ax, (%edi)

7:
	addl	$2, %esi
	addl	$2, %edi
	decl	%ecx
	jnz		innerl

	movl	12(%ebp), %ebx
	subl	40(%ebp), %ebx
	shll	$1, %ebx
	addl	%ebx, %edi

	movl	28(%ebp), %ebx
	subl	40(%ebp), %ebx
	shll	$1, %ebx
	addl	%ebx, %esi

	decl	%edx
	jnz		outerl

4:
	popl	%esi
	popl	%edi
	DelStack
	ret

.globl	SC_Trp_X
SC_Trp_X:
	CrtStack
	pushl	%edi
	pushl	%esi

	//x < lx ?
	movl	clp_lx, %eax
	cmpl	%eax, 16(%ebp)
	jge     0f
	movl	16(%ebp), %ebx
	subl	%ebx, %eax
	subl	%eax, 40(%ebp)
	addl	%eax, 32(%ebp)
	movl	clp_lx, %eax
	movl	%eax, 16(%ebp)
0:
	//y < ty ?
	movl	clp_ty, %eax
	cmp		%eax, 20(%ebp)
	jge     1f
	movl	20(%ebp), %ebx
	subl	%ebx, %eax
	subl	%eax, 44(%ebp)
	addl	%eax, 36(%ebp)
	movl	clp_ty, %eax
	movl	%eax, 20(%ebp)
1:
	//x > rx
    movl	clp_rx, %eax
    movl	16(%ebp), %ebx
    addl	40(%ebp), %ebx
    subl	$1, %ebx
    cmp		%eax, %ebx
    jle		2f
    movl	clp_rx, %ebx
    movl	16(%ebp), %eax
    subl	%eax, %ebx
    addl	$1, %ebx
    movl	%ebx, 40(%ebp)
2:
	//y > ty
    movl	clp_by, %eax
    movl	20(%ebp), %ebx
    addl	44(%ebp), %ebx
    subl	$1, %ebx
    cmp		%eax, %ebx
    jle		3f
    movl	clp_by, %ebx
    movl	20(%ebp), %eax
    subl	%eax, %ebx
    addl	$1, %ebx
    movl	%ebx, 44(%ebp)
3:
	
	//valid w?
	movl	40(%ebp), %eax
	cmpl	$0, %eax
	jle		4f
    //valid h?
	movl	44(%ebp), %eax
	cmpl	$0, %eax
	jle		4f

	xorl	%edi, %edi
	xorl	%esi, %esi
	xorl	%eax, %eax

	//esi source
	movl	36(%ebp), %eax //sy
	mull	28(%ebp) //dw
	addl	32(%ebp), %eax //sx
	shll	$1, %eax
	addl 	24(%ebp), %eax
	movl	%eax, %esi //source offset in esi

	//edi destination
	pushl	%ds
	popl	%es
	movl	20(%ebp), %eax
	mull	12(%ebp)
	addl	16(%ebp), %eax
	shll	$1, %eax
	addl	8(%ebp), %eax
	movl	%eax, %edi //dest offset in edi

	movl	44(%ebp), %edx
outerl4:
	movl	40(%ebp), %ecx
	pushl	%edx
innerl4:
	pushl	%ecx

	movw	(%esi), %bx
	cmpw	$0, %bx
	jz		7f

	movl	(%esi), %eax
	andl	rMask, %eax
	movb	rlShft, %cl
	shrl	%cl, %eax
	mull	48(%ebp)
	shll	%cl, %eax	
	pushl	%eax //relative red source part
	movl	(%edi), %eax
	andl	rMask, %eax
	shrl	%cl, %eax
	movl	$100, %ebx
	subl	48(%ebp), %ebx
	mull	%ebx
	shll	%cl, %eax //relative green dest part
	popl	%ebx
	addl	%ebx, %eax
	movl	$100, %ebx
	divl	%ebx
	andl	rMask, %eax
	pushl	%eax

	movl	(%esi), %eax
	andl	gMask, %eax
	movb	glShft, %cl
	shrl	%cl, %eax
	mull	48(%ebp)
	shll	%cl, %eax	
	pushl	%eax //relative red source part
	movl	(%edi), %eax
	andl	gMask, %eax
	shrl	%cl, %eax
	movl	$100, %ebx
	subl	48(%ebp), %ebx
	mull	%ebx
	shll	%cl, %eax //relative green dest part
	popl	%ebx
	addl	%ebx, %eax
	movl	$100, %ebx
	divl	%ebx
	andl	gMask, %eax
	pushl	%eax

	movl	(%esi), %eax
	andl	bMask, %eax
	movb	blShft, %cl
	shrl	%cl, %eax
	mull	48(%ebp)
	shll	%cl, %eax	
	pushl	%eax //relative blue source part
	movl	(%edi), %eax
	andl	bMask, %eax
	shrl	%cl, %eax
	movl	$100, %ebx
	subl	48(%ebp), %ebx
	mull	%ebx
	shll	%cl, %eax //relative blue dest part
	popl	%ebx
	addl	%ebx, %eax
	movl	$100, %ebx
	divl	%ebx
	andl	bMask, %eax

	popl	%ebx
	orl		%ebx, %eax
	popl	%ebx
	orl		%ebx, %eax

	movw	%ax, (%edi)

7:
	addl	$2, %esi
	addl	$2, %edi
	popl	%ecx
	decl	%ecx
	jnz		innerl4

	popl	%edx

	movl	12(%ebp), %ebx
	subl	40(%ebp), %ebx
	shll	$1, %ebx
	addl	%ebx, %edi

	movl	28(%ebp), %ebx
	subl	40(%ebp), %ebx
	shll	$1, %ebx
	addl	%ebx, %esi

	decl	%edx
	jnz		outerl4

4:
	popl	%esi
	popl	%edi
	DelStack
	ret

.globl	SC_Clr_11
SC_Clr_11:
	CrtStack
	pushl	%edi
	pushl	%esi

	//x < lx ?
	movl	clp_lx, %eax
	cmpl	%eax, 16(%ebp)
	jge     0f
	movl	16(%ebp), %ebx
	subl	%ebx, %eax
	subl	%eax, 40(%ebp)
	addl	%eax, 32(%ebp)
	movl	clp_lx, %eax
	movl	%eax, 16(%ebp)
0:
	//y < ty ?
	movl	clp_ty, %eax
 	cmp		%eax, 20(%ebp)
	jge     1f
	movl	20(%ebp), %ebx
	subl	%ebx, %eax
	subl	%eax, 44(%ebp)
	addl	%eax, 36(%ebp)
	movl	clp_ty, %eax
	movl	%eax, 20(%ebp)
1:
	//x > rx
    movl	clp_rx, %eax
    movl	16(%ebp), %ebx
    addl	40(%ebp), %ebx
    subl	$1, %ebx
    cmp		%eax, %ebx
    jle		2f
    movl	clp_rx, %ebx
    movl	16(%ebp), %eax
    subl	%eax, %ebx
    addl	$1, %ebx
    movl	%ebx, 40(%ebp)
2:
	//y > ty
    movl	clp_by, %eax
    movl	20(%ebp), %ebx
    addl	44(%ebp), %ebx
    subl	$1, %ebx
    cmp		%eax, %ebx
    jle		3f
    movl	clp_by, %ebx
    movl	20(%ebp), %eax
    subl	%eax, %ebx
    addl	$1, %ebx
    movl	%ebx, 44(%ebp)
3:
	
	//valid w?
	movl	40(%ebp), %eax
	cmpl	$0, %eax
	jle		4f
    //valid h?
	movl	44(%ebp), %eax
	cmpl	$0, %eax
	jle		4f

	xorl	%edi, %edi
	xorl	%esi, %esi
	xorl	%eax, %eax

	//esi source
	movl	36(%ebp), %eax //sy
	mull	28(%ebp) //dw
	addl	32(%ebp), %eax //sx
	shll	$1, %eax
	addl 	24(%ebp), %eax
	movl	%eax, %esi //source offset in esi

	//edi destination
	pushl	%ds
	popl	%es
	movl	20(%ebp), %eax
	mull	12(%ebp)
	addl	16(%ebp), %eax
	shll	$1, %eax
	addl	8(%ebp), %eax
	movl	%eax, %edi //dest offset in edi

	movl	44(%ebp), %edx //height
outerl7:
	movl	40(%ebp), %ecx //width
	pushl	%edx
innerl7:
	pushl	%ecx

	movw	(%esi), %bx
	cmpw	$0, %bx
	jz		7f

	xor		%ebx, %ebx
	movb	46(%ebp), %bl
	movb	rrShft, %cl
	shrb	%cl, %bl
	movb	rlShft, %cl
	shll	%cl, %ebx //relative red sourve value
	movl	(%edi), %eax
	andl	rMask, %eax
	addl	%ebx, %eax
	shrl	$1, %eax
	andl	rMask, %eax
	pushl	%eax

	xor		%ebx, %ebx
	movb	45(%ebp), %bl
	movb	grShft, %cl
	shrb	%cl, %bl
	movb	glShft, %cl
	shll	%cl, %ebx //relative green sourve value
	movl	(%edi), %eax
	andl	gMask, %eax
	addl	%ebx, %eax
	shrl	$1, %eax
	andl	gMask, %eax
	pushl	%eax

	xor		%ebx, %ebx
	movb	44(%ebp), %bl
	movb	brShft, %cl
	shrb	%cl, %bl
	movb	blShft, %cl
	shll	%cl, %ebx //relative blue sourve value
	movl	(%edi), %eax
	andl	bMask, %eax
	addl	%ebx, %eax
	shrl	$1, %eax
	andl	bMask, %eax

	popl	%ebx
	orl		%ebx, %eax
	popl	%ebx
	orl		%ebx, %eax

	movw	%ax, (%edi)

7:
	addl	$2, %edi
	addl	$2, %esi
	popl	%ecx
	decl	%ecx
	jnz		innerl7

	movl	28(%ebp), %eax
	subl	40(%ebp), %eax
	shll	$1, %eax
	addl	%eax, %esi

	movl	12(%ebp), %ebx
	subl	40(%ebp), %ebx
	shll	$1, %ebx
	addl	%ebx, %edi

	popl	%edx
	decl	%edx
	jnz		outerl7

4:
	popl	%esi
	popl	%edi
	DelStack
	ret

.globl	SC_Clr_X
SC_Clr_X:
	CrtStack
	pushl	%edi
	pushl	%esi

	//x < lx ?
	movl	clp_lx, %eax
	cmpl	%eax, 16(%ebp)
	jge     0f
	movl	16(%ebp), %ebx
	subl	%ebx, %eax
	subl	%eax, 40(%ebp)
	addl	%eax, 32(%ebp)
	movl	clp_lx, %eax
	movl	%eax, 16(%ebp)
0:
	//y < ty ?
	movl	clp_ty, %eax
	cmp		%eax, 20(%ebp)
	jge     1f
	movl	20(%ebp), %ebx
	subl	%ebx, %eax
	subl	%eax, 44(%ebp)
	addl	%eax, 36(%ebp)
	movl	clp_ty, %eax
	movl	%eax, 20(%ebp)
1:
	//x > rx
    movl	clp_rx, %eax
    movl	16(%ebp), %ebx
    addl	40(%ebp), %ebx
    subl	$1, %ebx
    cmp		%eax, %ebx
    jle		2f
    movl	clp_rx, %ebx
    movl	16(%ebp), %eax
    subl	%eax, %ebx
    addl	$1, %ebx
    movl	%ebx, 40(%ebp)
2:
	//y > ty
    movl	clp_by, %eax
    movl	20(%ebp), %ebx
    addl	44(%ebp), %ebx
    subl	$1, %ebx
    cmp		%eax, %ebx
    jle		3f
    movl	clp_by, %ebx
    movl	20(%ebp), %eax
    subl	%eax, %ebx
    addl	$1, %ebx
    movl	%ebx, 44(%ebp)
3:
	
	//valid w?
	movl	40(%ebp), %eax
	cmpl	$0, %eax
	jle		4f
    //valid h?
	movl	44(%ebp), %eax
	cmpl	$0, %eax
	jle		4f

	xorl	%edi, %edi
	xorl	%esi, %esi
	xorl	%eax, %eax

	//esi source
	movl	36(%ebp), %eax //sy
	mull	28(%ebp) //dw
	addl	32(%ebp), %eax //sx
	shll	$1, %eax
	addl 	24(%ebp), %eax
	movl	%eax, %esi //source offset in esi

	//edi destination
	pushl	%ds
	popl	%es
	movl	20(%ebp), %eax
	mull	12(%ebp)
	addl	16(%ebp), %eax
	shll	$1, %eax
	addl	8(%ebp), %eax
	movl	%eax, %edi //dest offset in edi

	movl	44(%ebp), %edx //height
outerl6:
	movl	40(%ebp), %ecx //width
	pushl	%edx
innerl6:
	pushl	%ecx

	movw	(%esi), %bx
	cmpw	$0, %bx
	jz		7f

	xor		%eax, %eax
	movb	50(%ebp), %al
	movb	rrShft, %cl
	shrb	%cl, %al
	mull	52(%ebp)
	movb	rlShft, %cl
	shll	%cl, %eax
	pushl	%eax //relative red source part
	movl	(%edi), %eax
	andl	rMask, %eax
	shrl	%cl, %eax
	movl	$100, %ebx
	subl	52(%ebp), %ebx
	mull	%ebx
	shll	%cl, %eax //relative red dest part
	popl	%ebx
	addl	%ebx, %eax
	movl	$100, %ebx
	divl	%ebx
	andl	rMask, %eax
	pushl	%eax

	xor		%eax, %eax
	movb	49(%ebp), %al
	movb	grShft, %cl
	shrb	%cl, %al
	mull	52(%ebp)
	movb	glShft, %cl
	shll	%cl, %eax
	pushl	%eax //relative green source part
	movl	(%edi), %eax
	andl	gMask, %eax
	shrl	%cl, %eax
	movl	$100, %ebx
	subl	52(%ebp), %ebx
	mull	%ebx
	shll	%cl, %eax //relative green dest part
	popl	%ebx
	addl	%ebx, %eax
	movl	$100, %ebx
	divl	%ebx
	andl	gMask, %eax
	pushl	%eax

	xor		%eax, %eax
	movb	48(%ebp), %al
	movb	brShft, %cl
	shrb	%cl, %al
	mull	52(%ebp)
	movb	blShft, %cl
	shll	%cl, %eax
	pushl	%eax //relative blue source part
	movl	(%edi), %eax
	andl	bMask, %eax
	shrl	%cl, %eax
	movl	$100, %ebx
	subl	52(%ebp), %ebx
	mull	%ebx
	shll	%cl, %eax //relative blue dest part
	popl	%ebx
	addl	%ebx, %eax
	movl	$100, %ebx
	divl	%ebx
	andl	bMask, %eax

	popl	%ebx
	orl		%ebx, %eax
	popl	%ebx
	orl		%ebx, %eax

	movw	%ax, (%edi)

7:
	addl	$2, %edi
	addl	$2, %esi
	popl	%ecx
	decl	%ecx
	jnz		innerl6

	movl	28(%ebp), %eax
	subl	40(%ebp), %eax
	shll	$1, %eax
	addl	%eax, %esi

	movl	12(%ebp), %ebx
	subl	40(%ebp), %ebx
	shll	$1, %ebx
	addl	%ebx, %edi

	popl	%edx
	decl	%edx
	jnz		outerl6

4:
	popl	%esi
	popl	%edi
	DelStack
	ret

.globl Gfx_SetClipRgn
Gfx_SetClipRgn:
	CrtStack

	movl	8(%ebp), %eax
	movl	%eax, clp_lx 		//left x
	movl	12(%ebp), %eax
	movl	%eax, clp_ty 		//top y
	movl	8(%ebp), %eax
	addl	16(%ebp), %eax
	subl	$1, %eax
	movl	%eax, clp_rx		//right x
	movl	12(%ebp), %eax
	addl	20(%ebp), %eax
	subl	$1, %eax
	movl	%eax, clp_by		//bottom y
	
	DelStack
	ret
