
main.o:     file format elf32-i386


Disassembly of section .text:

00000000 <waitdisk>:
   0:	ba f7 01 00 00       	mov    $0x1f7,%edx
   5:	ec                   	in     (%dx),%al
   6:	25 c0 00 00 00       	and    $0xc0,%eax
   b:	83 f8 40             	cmp    $0x40,%eax
   e:	75 f5                	jne    5 <waitdisk+0x5>
  10:	f3 c3                	repz ret 

00000012 <init_serial>:
  12:	ba f9 03 00 00       	mov    $0x3f9,%edx
  17:	b8 00 00 00 00       	mov    $0x0,%eax
  1c:	ee                   	out    %al,(%dx)
  1d:	b2 fb                	mov    $0xfb,%dl
  1f:	b8 80 ff ff ff       	mov    $0xffffff80,%eax
  24:	ee                   	out    %al,(%dx)
  25:	b2 f8                	mov    $0xf8,%dl
  27:	b8 01 00 00 00       	mov    $0x1,%eax
  2c:	ee                   	out    %al,(%dx)
  2d:	b2 f9                	mov    $0xf9,%dl
  2f:	b8 00 00 00 00       	mov    $0x0,%eax
  34:	ee                   	out    %al,(%dx)
  35:	b2 fb                	mov    $0xfb,%dl
  37:	b8 03 00 00 00       	mov    $0x3,%eax
  3c:	ee                   	out    %al,(%dx)
  3d:	b2 fa                	mov    $0xfa,%dl
  3f:	b8 c7 ff ff ff       	mov    $0xffffffc7,%eax
  44:	ee                   	out    %al,(%dx)
  45:	b2 fc                	mov    $0xfc,%dl
  47:	b8 0b 00 00 00       	mov    $0xb,%eax
  4c:	ee                   	out    %al,(%dx)
  4d:	c3                   	ret    

0000004e <serial_printc>:
  4e:	8b 4c 24 04          	mov    0x4(%esp),%ecx
  52:	ba fd 03 00 00       	mov    $0x3fd,%edx
  57:	ec                   	in     (%dx),%al
  58:	a8 20                	test   $0x20,%al
  5a:	74 fb                	je     57 <serial_printc+0x9>
  5c:	ba f8 03 00 00       	mov    $0x3f8,%edx
  61:	89 c8                	mov    %ecx,%eax
  63:	ee                   	out    %al,(%dx)
  64:	b2 fd                	mov    $0xfd,%dl
  66:	ec                   	in     (%dx),%al
  67:	a8 20                	test   $0x20,%al
  69:	74 fb                	je     66 <serial_printc+0x18>
  6b:	ba f8 03 00 00       	mov    $0x3f8,%edx
  70:	b8 0a 00 00 00       	mov    $0xa,%eax
  75:	ee                   	out    %al,(%dx)
  76:	c3                   	ret    

00000077 <readsect>:
  77:	56                   	push   %esi
  78:	53                   	push   %ebx
  79:	8b 5c 24 0c          	mov    0xc(%esp),%ebx
  7d:	8b 74 24 10          	mov    0x10(%esp),%esi
  81:	e8 fc ff ff ff       	call   82 <readsect+0xb>
  86:	6a 61                	push   $0x61
  88:	e8 fc ff ff ff       	call   89 <readsect+0x12>
  8d:	ba f2 01 00 00       	mov    $0x1f2,%edx
  92:	b8 01 00 00 00       	mov    $0x1,%eax
  97:	ee                   	out    %al,(%dx)
  98:	6a 41                	push   $0x41
  9a:	e8 fc ff ff ff       	call   9b <readsect+0x24>
  9f:	ba f3 01 00 00       	mov    $0x1f3,%edx
  a4:	89 f0                	mov    %esi,%eax
  a6:	ee                   	out    %al,(%dx)
  a7:	6a 42                	push   $0x42
  a9:	e8 fc ff ff ff       	call   aa <readsect+0x33>
  ae:	89 f0                	mov    %esi,%eax
  b0:	c1 f8 08             	sar    $0x8,%eax
  b3:	ba f4 01 00 00       	mov    $0x1f4,%edx
  b8:	ee                   	out    %al,(%dx)
  b9:	6a 43                	push   $0x43
  bb:	e8 fc ff ff ff       	call   bc <readsect+0x45>
  c0:	89 f0                	mov    %esi,%eax
  c2:	c1 f8 10             	sar    $0x10,%eax
  c5:	ba f5 01 00 00       	mov    $0x1f5,%edx
  ca:	ee                   	out    %al,(%dx)
  cb:	6a 44                	push   $0x44
  cd:	e8 fc ff ff ff       	call   ce <readsect+0x57>
  d2:	89 f0                	mov    %esi,%eax
  d4:	c1 f8 18             	sar    $0x18,%eax
  d7:	83 c8 e0             	or     $0xffffffe0,%eax
  da:	ba f6 01 00 00       	mov    $0x1f6,%edx
  df:	ee                   	out    %al,(%dx)
  e0:	6a 45                	push   $0x45
  e2:	e8 fc ff ff ff       	call   e3 <readsect+0x6c>
  e7:	ba f7 01 00 00       	mov    $0x1f7,%edx
  ec:	b8 20 00 00 00       	mov    $0x20,%eax
  f1:	ee                   	out    %al,(%dx)
  f2:	6a 62                	push   $0x62
  f4:	e8 fc ff ff ff       	call   f5 <readsect+0x7e>
  f9:	e8 fc ff ff ff       	call   fa <readsect+0x83>
  fe:	6a 63                	push   $0x63
 100:	e8 fc ff ff ff       	call   101 <readsect+0x8a>
 105:	89 d9                	mov    %ebx,%ecx
 107:	81 c3 00 02 00 00    	add    $0x200,%ebx
 10d:	83 c4 20             	add    $0x20,%esp
 110:	ba f0 01 00 00       	mov    $0x1f0,%edx
 115:	ed                   	in     (%dx),%eax
 116:	89 01                	mov    %eax,(%ecx)
 118:	83 c1 04             	add    $0x4,%ecx
 11b:	39 d9                	cmp    %ebx,%ecx
 11d:	75 f6                	jne    115 <readsect+0x9e>
 11f:	6a 64                	push   $0x64
 121:	e8 fc ff ff ff       	call   122 <readsect+0xab>
 126:	83 c4 04             	add    $0x4,%esp
 129:	5b                   	pop    %ebx
 12a:	5e                   	pop    %esi
 12b:	c3                   	ret    

0000012c <readseg>:
 12c:	57                   	push   %edi
 12d:	56                   	push   %esi
 12e:	53                   	push   %ebx
 12f:	8b 5c 24 10          	mov    0x10(%esp),%ebx
 133:	8b 74 24 18          	mov    0x18(%esp),%esi
 137:	89 df                	mov    %ebx,%edi
 139:	03 7c 24 14          	add    0x14(%esp),%edi
 13d:	89 f0                	mov    %esi,%eax
 13f:	c1 f8 1f             	sar    $0x1f,%eax
 142:	c1 e8 17             	shr    $0x17,%eax
 145:	01 c6                	add    %eax,%esi
 147:	89 f2                	mov    %esi,%edx
 149:	81 e2 ff 01 00 00    	and    $0x1ff,%edx
 14f:	29 c2                	sub    %eax,%edx
 151:	29 d3                	sub    %edx,%ebx
 153:	c1 fe 09             	sar    $0x9,%esi
 156:	83 c6 3f             	add    $0x3f,%esi
 159:	39 df                	cmp    %ebx,%edi
 15b:	76 17                	jbe    174 <readseg+0x48>
 15d:	56                   	push   %esi
 15e:	53                   	push   %ebx
 15f:	e8 fc ff ff ff       	call   160 <readseg+0x34>
 164:	81 c3 00 02 00 00    	add    $0x200,%ebx
 16a:	83 c6 01             	add    $0x1,%esi
 16d:	83 c4 08             	add    $0x8,%esp
 170:	39 df                	cmp    %ebx,%edi
 172:	77 e9                	ja     15d <readseg+0x31>
 174:	5b                   	pop    %ebx
 175:	5e                   	pop    %esi
 176:	5f                   	pop    %edi
 177:	c3                   	ret    

00000178 <bootmain>:
 178:	57                   	push   %edi
 179:	56                   	push   %esi
 17a:	53                   	push   %ebx
 17b:	6a 31                	push   $0x31
 17d:	e8 fc ff ff ff       	call   17e <bootmain+0x6>
 182:	6a 00                	push   $0x0
 184:	68 00 10 00 00       	push   $0x1000
 189:	68 00 80 00 00       	push   $0x8000
 18e:	e8 fc ff ff ff       	call   18f <bootmain+0x17>
 193:	6a 32                	push   $0x32
 195:	e8 fc ff ff ff       	call   196 <bootmain+0x1e>
 19a:	a1 1c 80 00 00       	mov    0x801c,%eax
 19f:	8d 98 00 80 00 00    	lea    0x8000(%eax),%ebx
 1a5:	0f b7 3d 2c 80 00 00 	movzwl 0x802c,%edi
 1ac:	c1 e7 05             	shl    $0x5,%edi
 1af:	01 df                	add    %ebx,%edi
 1b1:	83 c4 14             	add    $0x14,%esp
 1b4:	39 fb                	cmp    %edi,%ebx
 1b6:	73 3d                	jae    1f5 <bootmain+0x7d>
 1b8:	8b 43 0c             	mov    0xc(%ebx),%eax
 1bb:	8d b0 00 00 00 40    	lea    0x40000000(%eax),%esi
 1c1:	ff 73 04             	pushl  0x4(%ebx)
 1c4:	ff 73 10             	pushl  0x10(%ebx)
 1c7:	56                   	push   %esi
 1c8:	e8 fc ff ff ff       	call   1c9 <bootmain+0x51>
 1cd:	89 f0                	mov    %esi,%eax
 1cf:	03 43 10             	add    0x10(%ebx),%eax
 1d2:	89 f2                	mov    %esi,%edx
 1d4:	03 53 14             	add    0x14(%ebx),%edx
 1d7:	83 c4 0c             	add    $0xc,%esp
 1da:	39 d0                	cmp    %edx,%eax
 1dc:	73 10                	jae    1ee <bootmain+0x76>
 1de:	83 c0 01             	add    $0x1,%eax
 1e1:	c6 40 ff 00          	movb   $0x0,-0x1(%eax)
 1e5:	89 f2                	mov    %esi,%edx
 1e7:	03 53 14             	add    0x14(%ebx),%edx
 1ea:	39 d0                	cmp    %edx,%eax
 1ec:	72 f0                	jb     1de <bootmain+0x66>
 1ee:	83 c3 20             	add    $0x20,%ebx
 1f1:	39 df                	cmp    %ebx,%edi
 1f3:	77 c3                	ja     1b8 <bootmain+0x40>
 1f5:	6a 33                	push   $0x33
 1f7:	e8 fc ff ff ff       	call   1f8 <bootmain+0x80>
 1fc:	a1 18 80 00 00       	mov    0x8018,%eax
 201:	8d 98 00 00 00 40    	lea    0x40000000(%eax),%ebx
 207:	6a 40                	push   $0x40
 209:	e8 fc ff ff ff       	call   20a <bootmain+0x92>
 20e:	83 c4 08             	add    $0x8,%esp
 211:	ff d3                	call   *%ebx
 213:	5b                   	pop    %ebx
 214:	5e                   	pop    %esi
 215:	5f                   	pop    %edi
 216:	c3                   	ret    
