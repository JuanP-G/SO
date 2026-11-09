# Makefile de la Práctica 2
# -Wall -Wextra: avisos útiles; -g: símbolos para gdb/valgrind.
CC     = gcc
CFLAGS = -Wall -Wextra -g

PROGS = show_file show_file_getc write_strings read_strings \
        write_records_text read_records_text \
        write_records_bin read_records_bin \
        conversion c5_label_dinamico

all: $(PROGS)

# Regla genérica: cada programa se construye desde su .c homónimo.
# $@ = destino (el ejecutable)   $< = primer prerrequisito (el .c)
%: %.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(PROGS) *.o

# all y clean no son ficheros: .PHONY evita confundirlos con uno que se llame así
.PHONY: all clean
