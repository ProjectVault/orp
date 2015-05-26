{-|
Module: Main
Description: Home of the 'tidlgen' CLI entry point.

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
module Main where

import Control.Monad.Trans.Class
import Options

import Tidl.Cfg
import Tidl.Parser
import Tidl.Generate
import Tidl.Generate.C
import Tidl.Generate.Java

import qualified Data.Map.Strict as Map
import Text.PrettyPrint
import System.FilePath
import System.Directory

-- | 'main' runs the main program
main :: IO ()
main = runCommand $ \cfg leftover_args
 -> case validateCfg cfg leftover_args of
     Just cfgErr ->
      do putStrLn $ "Error with command-line args: " ++ show cfgErr
         putStrLn $ "Configuration: " ++ show cfg
     Nothing -> runCfg cfg start_tidlgen

start_tidlgen :: TidlCfg IO ()
start_tidlgen
 = do -- get the file contents
      maybe_infile <- infile
      input <- lift $ case maybe_infile of
                       Just fname -> readFile fname
                       Nothing -> getContents
      -- parse 'em
      case parseTidl input of
       Left err -> lift $ putStrLn $ "Parse error: " ++ show err
       Right ast -> -- on success, render 'em
        languages
        >>= mapM_ renderDoc . concatMap (generateOutput ast)

generateOutput ast (Java modname) = genAll TargetJava ast modname
generateOutput ast (C modname) = genAll TargetC ast modname

renderDoc :: Rendered -> TidlCfg IO ()
renderDoc (Rendered { path = fdir, fname = fname, contents = contents })
 = outdir >>= \odir -> lift $
    do let dir = odir </> foldl combine "" fdir
           fn  = dir </> fname
       createDirectoryIfMissing True dir
       writeFile fn (render contents)
       putStrLn $ "Wrote to file " ++ fn
