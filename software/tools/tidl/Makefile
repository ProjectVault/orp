all: cabal.sandbox.config
	cabal configure
	#cabal haddock --executables
	cabal build

sandbox: cabal.sandbox.config

cabal.sandbox.config: tidlgen.cabal
	cabal sandbox init
	cabal install --only-dependencies --enable-documentation

test:
	dist/build/tidlgen/tidlgen --java TIDL.Test \
	  --outdir examples/tidlout/java --infile examples/tidltest.tidl
	dist/build/tidlgen/tidlgen --C tidl_test  \
	  --outdir examples/tidlout/c --infile examples/tidltest.tidl

clean:
	rm -rf dist

