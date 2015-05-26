{-|
Module: Tidl.Generate
Description: Generation primitive for TIDL

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
module Tidl.Generate where

import Tidl.Cfg
import Tidl.Ast
import qualified Data.Map as Map

import Text.PrettyPrint (Doc)

data Rendered
 = Rendered
    { path :: [String]
    , fname :: String
    , contents :: Doc
    }

class GenTarget l where
 genAll :: l -> AST -> String -> [Rendered]

