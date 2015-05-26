########################################
#
# orp-or1k
#
#


ifndef OR1KTOOLS
$(error OR1KTOOLS environment variable not set; did you forget to source devenv-settings.sh?)
endif

include Tools.mk

include ORP.mk

all: all-tools all-orp

clean-tools:
	rm -rf build/tools

clean-orp:
	rm -rf build/orp

veryclean: clean
	rm -rf $(OR1KTOOLS) $(OR1KORP)
