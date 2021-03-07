
ACTIVATE=venv/bin/activate

.PHONY = .ACTIVATE_SOURCED

clean:
	rm -rf .ipynb_checkpoints/ "data-*.json"

clean-all: clean
	rm -rf venv/

install: .ACTIVATE_SOURCED
	. venv/bin/activate
	pip3 install -r requirements.txt

start: .ACTIVATE_SOURCED
	. venv/bin/activate
	jupyter notebook

.ACTIVATE_SOURCED: $(ACTIVATE)

$(ACTIVATE):
	python3 -m venv  venv

