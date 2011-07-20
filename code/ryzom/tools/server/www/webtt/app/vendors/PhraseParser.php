<?php
class PhraseParser
{
	var $debug = false;

	function removeComments($str)
	{
//As a pertinent note, there's an issue with this function where parsing any string longer than 94326 characters long will silently return null. So be careful where you use it at.
//http://pl.php.net/manual/en/function.preg-replace.php#98843
		ini_set('pcre.backtrack_limit', 10000000);
		//$returnString = preg_replace('!/\*.*?\*/!s', '', $str); // /* .*? */ s
		// added [^/] because there was //******* in translation file
		$returnString = preg_replace('![^/]/\*.*?\*/!s', '', $str); // /* .*? */ s
		// PHP 5.2.0
		// if (PREG_NO_ERROR !== preg_last_error())
		if ($returnString === null)
		{
			$returnStr = $str;
			var_dump("PREG ERROR");
			// exception
		}
		return $returnString;
	}

	function removeBOM($str)
	{
		if(($bom = substr($str, 0,3)) == pack("CCC",0xef,0xbb,0xbf))
		{
			$bla = substr($str, 3);
			return $bla;
		}
		else
		{
			return $str;
		}
	}

	function addBOM($str)
	{
		if(($bom = substr($str, 0,3)) != pack("CCC",0xef,0xbb,0xbf))
			return pack("CCC",0xef,0xbb,0xbf) . $str;
		else
			return $str;
	}

	function parseLine($str)
	{
		$arr = array();
		if (mb_substr($str, 0, 2) == "//")
		{
			if (mb_substr($str, 0, 7) == "// DIFF")
			{
				$tmp = explode(" ", $str);
				$type = 'diff';
				$command = mb_strtolower($tmp[2]);
				$args = isset($tmp[3]) ? $tmp[3] : null;
				if ($command == "add" || $command == "changed")
				{
					if (isset($args))
						$index = intval($args);
					else
						$index = null;
					$command = mb_substr($str, 3);
				}
				else
					unset($type);
			}
			else if (mb_substr($str, 0, 8) == "// INDEX")
			{
				list($j, $type, $index) = explode(" ", $str);
				$type = "internal_index";
			}
			else if (mb_substr($str, 0, 13) == "// HASH_VALUE")
			{
				list($j, $type, $hash_value) = explode(" ", $str);
				$type = "hash_value";
			}
/*			if (!isset($type))
			{
				var_dump(isset($type));
				debug_print_backtrace();
			}
			var_dump($type);*/
			if (isset($type))
			{
				$type = mb_strtolower($type);
				$arr = compact("type","command","index","hash_value");
			}
		}
		else if (!(mb_substr($str, 0, 2) == "//") && mb_strlen($str))
		{
			$type = "string";
			$string = "";
			$identifier = "";
			$arguments = "";
			$body = "";
			if (preg_match('|^([0-9A-Za-z@_]*) \((.*?)\).*$|', $str, $matches))
			{
				$identifier = $matches[1];
				$arguments = $matches[2];
			}
			else if ($str == "{")
				 $body = "begin";
			else if ($str == "}")
				$body = "end";
			else
			{
				$string = $str;
			}
			$arr = compact("type", "identifier", "arguments", "string", "body");
		}
/*		echo "<pre>################################\n";
		var_dump($str);
		var_dump($arr);
		echo "</pre>\n";*/
		return $arr;
	}

	function addEnt(&$parsedEnt, &$entities, &$newEnt)
	{
				if ($this->debug)
				{
					echo "\t%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n";
					echo "\t%%%% newEnt %%%%%%%%% newEnt %%%%%%%%% newEnt %%%%%%%%% newEnt %%%%%\n";
					echo "\t%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n\n";
					var_dump($parsedEnt);
				}
				if (!isset($parsedEnt["diff"]) && !isset($parsedEnt["index"]))
					$parsedEnt["index"] = $parsedEnt["internal_index"];

				$parsedEnt["type"] = "phrase";
				$entities[] = $parsedEnt;
				$parsedEnt =array();
				$newEnt = false;
	}

	function parseFile($file)
	{
		$parsedEnt = array();
		$newEnt = false;
		$prevStringLine = false;
		$entities = array();

		$file = $this->removeBOM($file);
//		var_dump($file);
		$file = $this->removeComments($file);
//		var_dump($file);
		$lines = explode("\n", $file);
		if ($this->debug)			
		{
			echo "<pre>\n\n";
		}
		$line_no=1;
		foreach ($lines as $line)
		{
			if ($this->debug)
			{
				echo "\n\t#################### LINE NUMBER " . $line_no++ . "\n\n";
				var_dump($line);
			}

			$line = rtrim($line, "\r\n");
			$parsedLine = $this->parseLine($line);

			if ($this->debug)
			{
				echo "%%%% parsedLine\n";
				var_dump($parsedLine);
				echo "\n";
			
				echo "%%%% prevStringLine\n";
				var_dump($prevStringLine);
				echo "\n";
			}

			if (!$parsedLine)
				continue;

			// if line start with diff (diff files) or hash_value (translated files) and before was line with translation, then we start new ent

			if ($prevStringLine && (
					($parsedLine["type"] == "diff" && $parsedEnt) || ($parsedLine["type"] == "hash_value" && $parsedEnt)
				))
			{
/*				echo "%%%% prevStringLine %%%%%\n";
				var_dump($parsedEnt);*/
				$newEnt = true;
			}

			if ($newEnt)
			{
				$this->addEnt($parsedEnt, $entities, $newEnt);
			}

			if ($parsedLine["type"] == "internal_index")
					$parsedEnt["internal_index"] = $parsedLine["index"];

			if ($parsedLine["type"] == "string")
			{
				$prevStringLine = true;

				if ($parsedLine["body"] == "begin" || $parsedLine["body"] == "end")
					continue;

				if ($this->debug)			
				{
					echo "%%%% parsedEnt before %%%%%\n";
					var_dump($parsedEnt);

//					echo "%%%% parsedLine %%%%%\n";
//					var_dump($parsedLine);
				}

				if (!$parsedLine['identifier'])
				{
					if ($this->debug) echo "%%%% parsedEnt ZLACZENIE \n";
					if ($this->debug && !isset($parsedEnt['string']))
					{
						echo "!isset parsedEnt['string']\n";
						var_dump($line);
						var_dump($parsedEnt);
						var_dump($parsedLine);
					}
					$parsedEnt['string'] .= $parsedLine['string'] . "\n";
				}
				else
				{
					if ($this->debug) echo "DODANIE \n";
					$parsedEnt += $parsedLine;
//					$parsedEnt['string'] .= "\n";
				}

				if ($this->debug)			
				{
					echo "%%%% parsedEnt after %%%%%\n";
					var_dump($parsedEnt);
				}
			}
			else
				$prevStringLine = false;

			if ($parsedLine["type"] == "diff")
			{
				$parsedEnt["diff"] = $parsedEnt["command"] = $parsedLine["command"];
				$parsedEnt["index"] = $parsedLine["index"];
			}
		}
		if ($parsedEnt)
		{
			$this->addEnt($parsedEnt, $entities, $newEnt);
		}

		if ($this->debug)			
		{
			echo "<pre>";
			var_dump($entities);
			echo "</pre>\n";
		}
		return $entities;
	}

	function CRLF($s)
	{
		$s = str_replace("\r\n", "\n", $s);
		$s = str_replace("\n", "\r\n", $s);
		return $s;
	}

	function buildFile($entities)
	{
		$content = '// DIFF_VERSION 2' . "\n";
		foreach ($entities as $ent)
		{
			if (isset($ent['command']))
				$content .= '// ' . $ent['command'] . "\n";
			if (isset($ent['hash_value']))
				$content .= '// HASH_VALUE ' . $ent['hash_value'];
/*			if (isset($ent['command']))
				$content .= '// INDEX ' . $ent['internal_index'] . "\n";
			else
				$content .= '// INDEX ' . $ent['index'] . "\n";*/
			$content .= $ent['identifier'] . ' (' . $ent['arguments'] . ')' . "\n" . '{' . "\n" . $ent['string'] . "\n" . '}' . "\n";
			$content .= "\n";
		}
		return $this->addBOM($this->CRLF($content));
	}
}
?>