#!/usr/bin/perl

open(SIG, $ARGV[0]) || die "open $ARGV[0]: $!";

$n = sysread(SIG, $buf, 1000);

if($n > 445){
  print STDERR "ERROR: boot block too large: $n bytes (max 445)\n";
  exit 1;
}

print STDERR "OK: boot block is $n bytes (max 445)\n";

$buf .= "\0" x (510-$n);
$buf .= "\x55\xAA";

open(SIG, ">$ARGV[0]") || die "open >$ARGV[0]: $!";
print SIG $buf;

open(SIG, $ARGV[1]) || die "open $ARGV[1]: $!";

$n = sysread(SIG, $buf, 8704);

print STDERR "Bootloader Stage 2 is $n bytes\n";

$buf .= "\0" x (8704-$n);

open(SIG, ">$ARGV[1]") || die "open >$ARGV[1]: $!";
print SIG $buf;

close SIG;
