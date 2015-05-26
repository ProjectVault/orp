{-|
Module: Tidl.Generate.Pretty
Description: Helpers for the pretty-printer

   Copyright 2015, Google Inc.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
-}
module Tidl.Generate.Pretty where

import Control.Monad.Trans.State.Strict
import Control.Applicative

import Text.PrettyPrint as PP

type Printer = State Doc

runPrinter :: Printer a -> PP.Doc
runPrinter f = snd (runState f PP.empty)

sayln :: Doc -> Printer ()
sayln d' = modify (\d -> d $+$ d')

sayln' :: String -> Printer()
sayln' s' = sayln (PP.text s')

in_braces :: Doc -> Doc
in_braces x = lbrace $+$ (nest 2 x) $+$ rbrace

in_parens :: Doc -> Doc
in_parens x = lparen <> (nest 2 x) <> rparen

vsepBy :: [Doc] -> Doc -> Doc
vsepBy xs s = vcat $ punctuate s (filter (not . isEmpty) xs)

sepBy :: [Doc] -> Doc -> Doc
sepBy xs s = hsep $ punctuate s (filter (not . isEmpty) xs)
