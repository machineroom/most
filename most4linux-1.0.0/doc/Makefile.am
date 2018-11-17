EXTRA_DIST = Doxyfile.driver Doxyfile.libnsadapt dox

.PHONY: doc
doc:
	doxygen Doxyfile.driver	
	doxygen Doxyfile.libnsadapt
clean-local:
	rm -rf most-kernel
	rm -rf libnsadapt
