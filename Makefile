EXE = hamming_code
EXE_ARGS = -v

all: hamming_code

run: hamming_code
	@cd bin && ./$(EXE) $(EXE_ARGS)

hamming_code:
	@cd src && $(MAKE)

format:
	@cd src && $(MAKE) format
	
clean:
	@cd src && $(MAKE) clean
	@rm -f ./bin/$(EXE)

help:
	@echo "Usage:"
	@echo -e " make [all]\t\tBuild $(EXE)"
	@echo -e " make run\t\tBuild and run the software"
	@echo -e " make format\t\tRun formatter on sources"
	@echo -e " make clean\t\tRemove all files generated by make"
	@echo -e " make help\t\tDisplay this help"

.PHONY: all run format clean help