<?php

#SYN:xvales03

ini_set('default_charset', 'UTF-8');
mb_internal_encoding('UTF-8');
mb_regex_encoding('UTF-8');

$format = null;
$instream = "php://stdin";
$outstream = "php://stdout";
$br = 0;

/* funkce vypisujici napovedu */
function print_help()
{
	echo "skript lze spustit s nasledujicimi parametry:
\	--format=filename \tparametr urcujici formatovaci soubor, volitelny
\	--input=filename \tparametr urcujici vstupni soubor, volitelny
\	\t\t\tpokud neni zadan vstup ocekavan na stdin
\	--output=filename \tparametr urcujici vystupni soubor, volitelny
\	\t\t\tpokud neni zadan vystup na stdout
\	--br \t\t\tpridani <br /> na konec kazdeho radku, volitelny\n";
	exit (0);
}

/* funkce rozhoduje zda $arg odpovida argumentu pro vypis napovedy */
function is_help($arg)
{
	if ($arg === "--help" || $arg === "-h")
		return true;
	return false;
}

/* funkce rozhoduje zda $arg odpovida argumentu pro pridani <br /> */
function is_br($arg)
{
	if ($arg === "--br")
		return true;
	return false;
}

/* funkce rozhoduje zda $arg odpovida argumentu zadavajicimu formatovaci soubor */
function is_format($arg)
{
	if (mb_substr ($arg, 0, 9) === "--format=")
		return true;
	return false;
}

/* funkce rozhoduje zda $arg odpovida argumentu zadavajicimu vstupni soubor */
function is_input($arg)
{
	if (mb_substr ($arg, 0, 8) === "--input=")
		return true;
	return false;
}

/* funkce rozhoduje zda $arg odpovida argumentu zadavajicimu vystupni soubor */
function is_output($arg)
{
	if (mb_substr ($arg, 0, 9) === "--output=")
		return true;
	return false;
}

/* funkce prochazi pres vsechny vstupni parametry $argv a u kazdeho urcuje
   o jaky parametr se jedna
   pokud nazari na nejaky parametr vic nez jednou, nebo jej nerozpozna
   vraci false */
function test_params($argc, $argv)
{
	global $format, $instream, $outstream, $br;	// globalni promenne do kterych se prizazuji nazvy souboru

	$isfor = 0;
	$isin = 0;
	$isout = 0;

	for ($i = 1; $i < $argc; $i++)
	{
		if (is_help($argv[$i]) && $argc == 2)
		{
			print_help();
		}
		else if (is_br($argv[$i]) && $br == 0)
		{
			$br++;
		}
		else if (is_format($argv[$i]) && $isfor == 0)
		{
			$format = mb_substr($argv[$i], mb_strpos($argv[$i], '=') + 1);		// prirazeni nazvu souboru - cast argumentu za =
			$isfor++;
		}
		else if (is_input($argv[$i]) && $isin == 0)
		{
			$instream = mb_substr($argv[$i], mb_strpos($argv[$i], '=') + 1);	// prirazeni nazvu souboru - cast argumentu za =
			$isin++;
		}
		else if (is_output($argv[$i]) && $isout == 0)
		{
			$outstream = mb_substr($argv[$i], mb_strpos($argv[$i], '=') + 1);	// prirazeni nazvu souboru - cast argumentu za =
			$isout++;
		}
		else
			return false;
	}
	return true;
}

/* funkce testuje jestli se podarilo otevrit vstupni a formatovaci soubor
   chyba pri otevirani formatovaciho souboru neukonci program, ale
   formatovaci soubor bude nahrazen prazdnym retezcem */
function test_streams($instream, &$format)
{
	if ($instream === false)
	{
		return false;
	}

	if ($format === false)
	{
		$format = "";
	}
	return true;
}

/* nacteni jednoho radku formatovaciho souboru, rozdeli radek na cast 
   podporuje odradkovani \n a \r\n */
function get_line(&$format)
{
	if (mb_strlen($format) === 0)
		return false;
	
	$pos = mb_strpos($format, "\r");	
	if ($pos !== false)
		$format = mb_substr($format, 0, $pos) . mb_substr($format, $pos + 1);
	
	$pos = mb_strpos($format, "\n");
	
	if ($pos === false)
		$pos = mb_strlen($format);
	
	$line = mb_substr($format, 0, $pos);
	$format = mb_substr($format, $pos + 1);
	return $line;
}

/* rozdeleni radku formatovaciho souboru, cast regularniho vyrazu je ulozena
   do $regex a seznam tagu do $tags */
function parse_line($line, &$regex, &$tags)
{
	$pos = mb_strpos($line, "\t");		// $pos konec regularniho vyrazu
	if ($pos === false)
	{
		fwrite(STDERR, "CHYBA: spatny format formatovaciho souboru!\n");
		exit (4);
	}
	
	$regex = mb_substr($line, 0, $pos);
	$regex = "(" . $regex . ")";
	$line = mb_substr($line, $pos);
	
	while ((mb_strlen($line) !== 0) && (mb_substr($line, 0, 1) === "\t"))	// odstraneni tabulatoru pred tagy
	{
		$line = mb_substr($line, 1);
	}
	
	if (mb_strlen($line) === 0)
	{
		fwrite(STDERR, "CHYBA: spatny format formatovaciho souboru!\n");
		exit (4);
	}
	
	$tags = $line;
}

/* urcuje zda je potreba pouzit escape sekvenci pro zapsani znaku */
function is_escape($char)
{
	if ($char === "\\" || $char === "^" || $char === "$" || $char === "[" || $char === "]" || $char === "{" || $char === "}" || $char === "?" || $char === "~")
		return true;
	return false;
}

/* urcuje zda je potreba pouzit escape sekvenci pro zapsani znaku (znaky po %) */
function is_escape_2($char)
{
	if ($char === "." || $char === "|" || $char === "(" || $char === ")" || $char === "*" || $char === "+")
		return true;
	return false;
}

/* nacteni jednoho znaku neohraniceneho zavorkami, pouzivano pro negaci jednoho
   symbolu nebo escape sekvence
   vraci null pokud nebylo mozne nacist znak */
function read_one($regex, &$pos)
{
	$state = "read";
	$newex = "";
	$empty = true;
	
	while ($state !== "fin")
	{
		/*echo "newex: $newex\n";*/
		switch ($state)
		{
			case "read":	// pocatecni stav, ocekava znak nebo pocatek escape sekvence %
				if ($pos >= mb_strlen($regex))
				{
					return null;
				}
				
				$aux = mb_substr($regex, $pos, 1);
				if (is_escape($aux))	// pred znak je potreba vlozit \ 
				{					
					$newex = $newex . "\\" . $aux;
					$empty = false;
					$pos++;
					$state = "fin";
				}
				else if ($aux === "%")	// pocatek escape sekvence
				{
					$pos++;
					$state = "escaped";
				}
				else if ($aux !== "+" && $aux !== "*" && $aux !== "." && $aux !== "|"  && $aux !== ")") // symboly, ktere se nemohou za negaci vyskytnout
				{
					$newex = $newex . $aux;
					$pos++;
					$empty = false;
					$state = "fin";
				}
				else
				{
					return null;
				}
				break;				
				
			case "escaped":	// znaky vyskytujici se po escepe sekvenci a jejich prevody na format se kterym pracuje preg_match
				if ($pos >= mb_strlen($regex))
				{
					return null;
				}
				
				if (is_escape_2(mb_substr($regex, $pos, 1)))
				{
					$newex = $newex . "\\" . mb_substr($regex, $pos, 1);
					$empty = false;
					$pos++;
					$state = "fin";
					
				}
				else if (mb_substr($regex, $pos, 1) === "!" || mb_substr($regex, $pos, 1) === "%")
				{
					$newex = $newex . mb_substr($regex, $pos, 1);
					$empty = false;
					$pos++;
					$state = "fin";
				}
				else if (mb_substr($regex, $pos, 1) === "t")
				{
					$newex = $newex . "\\t";
					$empty = false;
					$pos++;
					$state = "fin";
				}
				else if (mb_substr($regex, $pos, 1) === "n")
				{
					$newex = $newex . "\\n";
					$empty = false;
					$pos++;
					$state = "fin";
				}
				else if (mb_substr($regex, $pos, 1) === "s")
				{
					$newex = $newex . "\s";
					$empty = false;
					$pos++;
					$state = "fin";
				}
				else if (mb_substr($regex, $pos, 1) === "a")
				{
					$newex = $newex . "[\s|\S]";
					$empty = false;
					$pos++;
					$state = "fin";
				}
				else if (mb_substr($regex, $pos, 1) === "d")
				{
					$newex = $newex . "\d";
					$empty = false;
					$pos++;
					$state = "fin";
				}
				else if (mb_substr($regex, $pos, 1) === "l")
				{
					$newex = $newex . "[a-z]";
					$empty = false;
					$pos++;
					$state = "fin";
				}
				else if (mb_substr($regex, $pos, 1) === "L")
				{
					$newex = $newex . "[A-Z]";
					$empty = false;
					$pos++;
					$state = "fin";
				}
				else if (mb_substr($regex, $pos, 1) === "w")
				{
					$newex = $newex . "[a-zA-Z]";
					$empty = false;
					$pos++;
					$state = "fin";
				}
				else if (mb_substr($regex, $pos, 1) === "W")
				{
					$newex = $newex . "[a-zA-Z0-9]";
					$empty = false;
					$pos++;
					$state = "fin";
				}
				else
				{
					return null;
				}
				break;			
		}
	}
	return $newex;
}

/* prevedeni regularniho vyrazu na format se kterym pracuje preg_match
   predpoklada retezec ohraniceny zavorkami
   rekurzivne volano pro podretezce uzavrene v zavorkach
   vraci null pokud nebylo mozne nacist znak */
function convert_regex($regex, &$pos)
{
	$state = "init";		// pocatecni stav automatu
	$newex = "";			// sem se uklada prevedeny regularni vyraz
	$empty = true;	
	$last_escaped = 0;		// urcuje zda posledni znak byl escape sekvence
	
	
	while ($state !== "fin")
	{
		if ($last_escaped > 0)
			$last_escaped--;
		/*echo "newex: $newex\n";*/
		$aux = mb_substr($regex, $pos);
		switch ($state)
		{
			case "init":	// prvni znak musi byt '('
				if ($pos < mb_strlen($regex) && mb_substr($regex, $pos, 1) === "(")
				{
					$newex = $newex . "(";
					$pos++;
					$state = "read";
				}
				else
					return null;
				break;
				
			case "read":	// zakladni stav pro cteni
				if ($pos >= mb_strlen($regex))
				{
					return null;
				}
				
				if (is_escape(mb_substr($regex, $pos, 1)))	// nasledujici znak je potreba dat do escape sekvence
				{					
					$newex = $newex . "\\" . mb_substr($regex, $pos, 1);
					$empty = false;
					$pos++;
				}
				else if (mb_substr($regex, $pos, 1) === ".")	// konkatenace, pred timto znakem se nesmi vyskytovat symboly se specialnim vyznamem
				{
					$aux = mb_substr($regex, $pos - 1, 1);
					if (($aux !== "." && $aux !== "|" && $aux !== "!" && $aux !== "(") || $last_escaped === 1)
					{
						$pos++;
					}
					else
					{
						return null;
					}
				}
				else if (mb_substr($regex, $pos, 1) === "|")	// "nebo", pred timto znakem se nesmi vyskytovat symboly se specialnim vyznamem
				{
					$aux = mb_substr($regex, $pos - 1, 1);
					if (($aux !== "." && $aux !== "|" && $aux !== "!" && $aux !== "(") || $last_escaped === 1)
					{
						$newex = $newex . "|";
						$pos++;
					}
					else
					{
						return null;
					}
				}
				else if (mb_substr($regex, $pos, 1) === "+" || mb_substr($regex, $pos, 1) === "*")	// iterace a pozitivni iterace, pred timto znakem se nesmi vyskytovat symboly se specialnim vyznamem
				{
					$aux = mb_substr($regex, $pos - 1, 1);
					if (($aux !== "." && $aux !== "|" && $aux !== "!" && $aux !== "(" && $aux !== "+" && $aux !== "*") || $last_escaped === 1)
					{
						$newex = $newex . mb_substr($regex, $pos, 1);
						$pos++;
					}
					else
					{
						return null;
					}
				}
				else if (mb_substr($regex, $pos, 1) === "%")	// nasleduje escape sekvence
				{
					$pos++;
					$state = "escaped";
				}
				else if (mb_substr($regex, $pos, 1) === "!")	// nasleduje negace
				{
					$pos++;
					$state = "negate";
				}
				else if (mb_substr($regex, $pos, 1) === "(")	// nasleduje vyraz uzavreny v zavorkach, rekurzivne volej convert_regex()
				{
					if (($aux = convert_regex($regex, $pos)) === null)
						return null;
					$newex = $newex . $aux;
					$empty = false;
				}
				else if (mb_substr($regex, $pos, 1) === ")")	// konec vyrazu uravreneho v zavorkach, nesmi mu predchazet symboly se specialnim vyznamem
				{
					$aux = mb_substr($regex, $pos - 1, 1);
					if ((($aux !== "." && $aux !== "|" && $aux !== "!") || $last_escaped === 1) && !$empty)
					{
						$newex = $newex . ")";
						$pos++;
						$state = "fin";
					}
					else
					{
						return null;
					}
				}
				else // ostatni symboly
				{
					$newex = $newex . mb_substr($regex, $pos, 1);
					$pos++;
					$empty = false;
					$state = "read";
				}
				break;				
				
			case "escaped":	// escape sekvence a jejich konverze na odpovidijici vyrazy pro preg_match
				if ($pos >= mb_strlen($regex))
				{
					return null;
				}
				
				if (is_escape_2(mb_substr($regex, $pos, 1)))
				{
					$newex = $newex . "\\" . mb_substr($regex, $pos, 1);
					$empty = false;
					$pos++;
					$state = "read";
					$last_escaped = 2;
				}
				else if (mb_substr($regex, $pos, 1) === "!" || mb_substr($regex, $pos, 1) === "%")
				{
					$newex = $newex . mb_substr($regex, $pos, 1);
					$empty = false;
					$pos++;
					$state = "read";
					$last_escaped = 2;
				}
				else if (mb_substr($regex, $pos, 1) === "t")
				{
					$newex = $newex . "\\t";
					$empty = false;
					$pos++;
					$state = "read";
				}
				else if (mb_substr($regex, $pos, 1) === "n")
				{
					$newex = $newex . "\\n";
					$empty = false;
					$pos++;
					$state = "read";
				}
				else if (mb_substr($regex, $pos, 1) === "s")
				{
					$newex = $newex . "\s";
					$empty = false;
					$pos++;
					$state = "read";
				}
				else if (mb_substr($regex, $pos, 1) === "a")
				{
					$newex = $newex . "[\s|\S]";
					$empty = false;
					$pos++;
					$state = "read";
				}
				else if (mb_substr($regex, $pos, 1) === "d")
				{
					$newex = $newex . "\d";
					$empty = false;
					$pos++;
					$state = "read";
				}
				else if (mb_substr($regex, $pos, 1) === "l")
				{
					$newex = $newex . "[a-z]";
					$empty = false;
					$pos++;
					$state = "read";
				}
				else if (mb_substr($regex, $pos, 1) === "L")
				{
					$newex = $newex . "[A-Z]";
					$empty = false;
					$pos++;
					$state = "read";
				}
				else if (mb_substr($regex, $pos, 1) === "w")
				{
					$newex = $newex . "[a-zA-Z]";
					$empty = false;
					$pos++;
					$state = "read";
				}
				else if (mb_substr($regex, $pos, 1) === "W")
				{
					$newex = $newex . "[a-zA-Z0-9]";
					$empty = false;
					$pos++;
					$state = "read";
				}
				else
				{
					return null;
				}
				break;
				
			case "negate":	// nasleduje negovany vyraz
				if (mb_substr($regex, $pos, 1) === "!")			// negace negace je ignorovana
				{
					$pos++;
					$state = "read";
				}
				else if (mb_substr($regex, $pos, 1) === "(")	// nasleduje vyraz v zavorce, rekurzivni volani convert_regex()
				{
					if (($aux = convert_regex($regex, $pos)) === null)
						return null;
					$newex = $newex . "(?:(?!" . $aux . ")[\s|\S])";
					$empty = false;
					$state = "read";
				}
				else											// nasleduje jediny znak()
				{
					if (($aux = read_one($regex, $pos)) === null)
						return null;
					$newex = $newex . "(?:(?!" . $aux . ")[\s|\S])";
					$empty = false;
					$state = "read";
				}
				break;				
		}
	}
	return $newex;
}

/* odstranei bilych znaku a carky mezi tagy */
function remove_space(&$tags)
{
	$len = mb_strlen($tags);
	if (mb_substr($tags, 0 , 1) !== ",")
		return false;
	$tags = mb_substr($tags, 1);
	$len--;
	while ($len !== 0 && (mb_substr($tags, 0 , 1) === "\t" || mb_substr($tags, 0 , 1) === " "))
	{
		$tags = mb_substr($tags, 1);
		$len--;		
	}
	if ($len === 0)
		return false;
	return true;
}

/* rozpoznani jednotlivych tagu a ulozeni jejich oteviracich a zaviracich
   retezcu do $open a $close */
function parse_tags($tags, &$open, &$close)
{
	$len = mb_strlen($tags);
	if ($len === 0)
		return false;
	$b = 0;	// kazdy tag se muze na radku vyskytnout jen jednou
	$i = 0;
	$u = 0;
	$tt = 0;
	$sz = 0;
	$txt = 0;
	while ($len !== 0)
	{
		$len = mb_strlen($tags);
		if ($len >= 4 && $b == 0 && mb_substr($tags, 0, 4) === "bold")	// tag bold
		{
			$b++;
			$open = $open . "<b>";
			$close = "</b>" . $close;
			if ($len == 4)		// tag je posledni
				return true;
			$tags = mb_substr($tags, 4);
			$len = mb_strlen($tags);
			
			if (!remove_space($tags))
				return false;
		}
		else if ($len >= 6 && $sz == 0 && mb_substr($tags, 0, 5) === "size:" && mb_substr($tags, 5, 1) >= "1" && mb_substr($tags, 5, 1) <= "7")	// tag size
		{
			$sz++;
			$open = $open . "<font size=" . mb_substr($tags, 5, 1) . ">";
			$close = "</font>" . $close;
			if ($len == 6)		// tag je posledni
				return true;
			$tags = mb_substr($tags, 6);
			$len = mb_strlen($tags);
			
			if (!remove_space($tags))
				return false;
		}
		else if ($len >= 6 && $i == 0 && mb_substr($tags, 0, 6) === "italic")	// tag italic
		{
			$i++;
			$open = $open . "<i>";
			$close = "</i>" . $close;
			if ($len == 6)		// tag je posledni
				return true;
			$tags = mb_substr($tags, 6);
			$len = mb_strlen($tags);
			
			if (!remove_space($tags))
				return false;
		}
		else if ($len >= 8 && $tt == 0 && mb_substr($tags, 0, 8) === "teletype")	// tag teletype
		{
			$tt++;
			$open = $open . "<tt>";
			$close = "</tt>" . $close;
			if ($len == 8)		// tag je posledni
				return true;
			$tags = mb_substr($tags, 8);
			$len = mb_strlen($tags);
			
			if (!remove_space($tags))
				return false;
		}
		else if ($len >= 9 && $u == 0 && mb_substr($tags, 0, 9) === "underline")	// tag underline
		{
			$u++;
			$open = $open . "<u>";
			$close = "</u>" . $close;
			if ($len == 9)		// tag je posledni
				return true;
			$tags = mb_substr($tags, 9);
			$len = mb_strlen($tags);
			
			if (!remove_space($tags))
				return false;
		}
		else if ($len >= 12 && $txt == 0 && mb_substr($tags, 0, 6) === "color:")	// tag color
		{
			$txt++;
			if (preg_match("~[a-fA-F0-9]{6}~", mb_substr($tags, 6, 6)) === 0)	// overeni formatu barvy
			{
				fwrite(STDERR, "CHYBA: spatny format formatovaciho souboru!\n");
				exit (4);
			}
			$open = $open . "<font color=#" . mb_substr($tags, 6, 6) . ">";
			$close = "</font>" . $close;
			if ($len === 12)		// tag je posledni
				return true;
			$tags = mb_substr($tags, 12);
			$len = mb_strlen($tags);
			
			if (!remove_space($tags))
				return false;
		}
		else	// tag neodpovida zadne moznosti
			return false;
	}
}

/* funkce prepocita pozice pro vlozeni oteviraci a zaviraci tagu
   s ohledem na jiz vlozene tagy, delky vlozenych tagu se ukladaji
   do $offset */
function align_pos(&$ofset, &$pos, &$cpos)
{
	$konstpos = $pos;
	$konstcpos = $cpos;
	for ($i = 0; $i <= $konstpos; $i++)
		$pos += $ofset[$i];
	
	for ($i = 0; $i < $konstcpos; $i++)
		$cpos += $ofset[$i];
}

/* funkce vlozi prislusne oteviraci a zaviraci tagy na pozici ve 
   vystupnim retezci $output, prochazi celym retezcem dokud existuje
   podretezec odpovidajici danemu regularnimu vyrazu nebo nebyl
   dosazen konec retezce, informace o vlozeni tagu uklada do $ofset */
function aplly_tags($open, $close, $regex, $instream, &$ofset, &$output)
{
	$matches;
	$olen = mb_strlen($open);	// delka tagu
	$clen = mb_strlen($close);
	$pos = preg_match($regex, $instream, $matches);		// pozice odpovidajici podretezce
	if ($pos === false)
	{
		fwrite(STDERR, "CHYBA: interni!\n");
		exit (100);	
	}
	$cutout = 0;	// soucasna pozice v prohledavanem retezci
	
	while ($pos === 1 && 0 < mb_strlen($instream))
	{		
		/*echo "match:\n@$matches[0]@\n";
		echo "instream:\n@$instream@\n";*/
		$len = mb_strlen($matches[0]);
		if ($len !== 0)
		{			
			$pos = mb_strpos($instream, $matches[0]);	// nalezeni pozice na ketre zacina podretezec
			$pos_orig = $pos + $cutout;	// pozice v puvodnim vstupu (posuna o jiz projitou cast retezce)
			$epos_orig = $pos + $cutout + $len;
			$kpos_orig = $pos_orig;	// pozice ke kterym nebude prepocitany podle vlozenych tagu
			$kepos_orig = $epos_orig;
			align_pos($ofset, $pos_orig, $epos_orig);
			
			$output = mb_substr($output, 0, $pos_orig) . $open . mb_substr($output, $pos_orig, $epos_orig - $pos_orig) . $close . mb_substr($output, $epos_orig);
			
			$ofset[$kpos_orig] += $olen;	// aktualizace ofsetu o nove vlozene tagy
			$ofset[$kepos_orig] += $clen;
			
			/*echo "output:\n@$output@\n";*/
			
			$instream = mb_substr($instream, $pos + $len);	// oriznuti vstupu o jiz osetrenou cast
			$cutout += $pos + $len;
			
			/*$ppos = $pos;
			$pcpos = $pos + $len;
			$pos += $cutout;
			$cpos = $pos + $len;
			
			$output = mb_substr($output, 0, $pos) . $open . mb_substr($output, $pos, $cpos - $pos) . $close . mb_substr($output, $cpos);
			$ofset[$ppos] += $olen;
			$ofset[$pcpos] += $clen;*/
		}
		else	// osetreni prazdych retezcu
		{
			$instream = mb_substr($instream, 1);
			$cutout++;
		}
		$pos = preg_match($regex, $instream, $matches);
		if ($pos === false)
		{
			fwrite(STDERR, "CHYBA: interni!\n");
			exit (100);
		}
	}
	return;
}

/* funkce prida tagy pozadovane --br */
function add_br(&$output)
{
	$output = preg_replace("~\n~", "<br />\n", $output);
}

/**************
 telo programu
***************	*/

if (!test_params($argc, $argv))	// testovani validity parametru
{
	fwrite(STDERR, "CHYBA: spatne parametry!\n");
	exit (1);	
}

$instream = @file_get_contents($instream);		// nacteni vstupniho a formatovaci souboru
$format = @file_get_contents($format);

if (!test_streams($instream, $format))	// testovani existence vstupu
{
	fwrite(STDERR, "CHYBA: nelze otevrit vstupni soubor!\n");
	exit (2);	
}

$output = $instream;
$ofset = array_fill(0, mb_strlen($instream) + 1, 0);	// pole pro ukladani delek tagu vlozenych na jednotlive pozice

$line = get_line($format);
while ($line !== false)		// pruchod formatovacim souborem radek po radku
{
	if ($line !== "")
	{
		$pos = 0;
		$regex;
		$tags;
		parse_line($line, $regex, $tags);		// rodeleni regularniho vyrazu a tagu
		$ln = mb_strlen($regex);
		$regex = convert_regex($regex, $pos);	// prevedeni regularniho vyrazu
		if ($regex === null || $pos != $ln)
		{
			fwrite(STDERR, "CHYBA: spatny format formatovaciho souboru!\n");
			exit (4);
		}		
		$regex = "~" . $regex . "~";		// obaleni regularniho vyrazu
		$open = "";
		$close = "";
		if (!parse_tags($tags, $open, $close))	// prevedeni tagu
		{
			fwrite(STDERR, "CHYBA: spatny format formatovaciho souboru!\n");
			exit (4);
		}
		/*echo "regex: $regex\n";*/
		/*echo "open: $open\n";
		echo "close: $close\n";*/
		aplly_tags($open, $close, $regex, $instream, $ofset, $output);	// vlozeni tagu na prislisne pozice
	}	
	$line = get_line($format);
}

if ($br === 1)		// pridani <br \> tagu
	add_br($output);

if (@file_put_contents($outstream, $output) === false)	// zapis vysledku
{
	fwrite(STDERR, "CHYBA: nelze zapsat vystup!\n");
	exit (3);
}
?>
