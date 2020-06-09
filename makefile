test: face.mdl main.py matrix.py mdl.py display.py draw.py gmath.py
	python main.py face.mdl

lint:
ifneq (, $(shell which flake8))
	flake8 ./graphics-engine --count --select=E9,F63,F7,F82 --show-source --statistics
endif

format:
ifneq (, $(shell which yapf))
	yapf -r -i --style pep8 -p -vv ./graphics-engine
endif

clean:
	rm *pyc *out parsetab.py

clear:
	rm *pyc *out parsetab.py *ppm