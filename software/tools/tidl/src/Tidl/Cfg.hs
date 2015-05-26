{-|
Module: Tidl.Cfg
Description: Run-time configuration for tidlgen

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
module Tidl.Cfg where

import Control.Monad.Trans.Reader
import Control.Applicative
import Options

data Language
  = Java String
  | C String
 deriving Show

data Cfg = Cfg
  { cfg_outdir :: String
  , cfg_emitJava :: Maybe String
  , cfg_emitC :: Maybe String
  , cfg_infile :: Maybe String
  }
 deriving Show

targetLanguages :: Cfg -> [Language]
targetLanguages cfg
  = let j = case cfg_emitJava cfg of
              Nothing -> []
              Just s -> [Java s]
        c = case cfg_emitC cfg of
              Nothing -> []
              Just s -> [C s]
    in j ++ c

data CfgError
  = NoTargetLang
  | NoOutDir
  | UnexpectedArgs [String]
 deriving Show

validateCfg :: Cfg -> [String] -> Maybe CfgError
validateCfg cfg []
 | null (targetLanguages cfg) = Just NoTargetLang
 | otherwise = Nothing
validateCfg _ leftovers = Just $ UnexpectedArgs leftovers

instance Options Cfg where
  defineOptions = pure Cfg
    <*> simpleOption "outdir" "./" "Output base path"
    <*> simpleOption "java" Nothing "Emit Java"
    <*> simpleOption "C" Nothing "Emit C"
    <*> simpleOption "infile" Nothing "Input file (stdin if omitted)"

type TidlCfg m = ReaderT Cfg m

runCfg :: Monad m => Cfg -> TidlCfg m a -> m a
runCfg cfg a = runReaderT a cfg

languages :: Monad m => TidlCfg m [Language]
languages = ask >>= return . targetLanguages

infile :: Monad m => TidlCfg m (Maybe String)
infile = ask >>= return . cfg_infile

outdir :: Monad m => TidlCfg m (String)
outdir = ask >>= return . cfg_outdir
