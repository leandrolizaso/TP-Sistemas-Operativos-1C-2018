pelao: init_deps install_lib make_projects

init_deps:
	@echo "Fetchig dependecies from github"
	@git submodule init && git submodule update
	@echo "Installing SO-COMMONS-LIBRARY"
	@cd commons && sudo $(MAKE) install
	@echo "Installing PARSI"
	@cd parsi && sudo $(MAKE) install

install_lib:
	@echo "Installing PELAO shared library"
	@cd lib && sudo $(MAKE) install

make_projects:
	@echo "Building all projects"
	@cd Coordinador && $(MAKE)
	@cd Planificador && $(MAKE)
	@cd ESI && $(MAKE)
	@cd Instancia && $(MAKE)
	@echo "Disfruta del Pelao!"

.PHONY: pelao init_deps install_lib make_projects

