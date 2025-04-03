# NIA de los autores
NIA1 = 100495956
NIA2 = 100495810
NIA3 = 100495746

ZIP = ssoo_p2_$(NIA1)_$(NIA2)_$(NIA3).zip
FILES = ../scripter.c ../mygrep.c ../Makefile ../autores.txt
TESTER_DIR = ssoo_p2_tester
TESTER = $(ZIP)

# Crear el ZIP directamente DENTRO de ssoo_p2_tester/
zip:
	cd $(TESTER_DIR) && zip $(ZIP) $(FILES)

# Ejecutar test 1 desde ssoo_p2_tester/
test1: zip
	cd $(TESTER_DIR) && ./tester.sh $(ZIP) 1

# Ejecutar test 2 desde ssoo_p2_tester/
test2: zip
	cd $(TESTER_DIR) && ./tester.sh $(ZIP) 2

