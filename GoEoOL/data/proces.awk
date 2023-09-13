BEGIN {
    square=0
    curly=0
    FS=""
}


{
    gsub(/^[[:space:]]+/, "")  # Usuń białe znaki na początku linii

    for (i = 1; i <= NF; i++) {
	if ($i == "}") {
    	    curly--
	} else if ($i == "]") {
    	    square--
	}
	}
    
    for (i = 1; i <= square ; i++) {
	printf "  "  # Dodaj spacje zgodnie z wartością 'square'
    }
    for (i = 1; i <= curly ; i++) {
	printf "   "  # Dodaj spacje zgodnie z wartością 'curly'
    }
    print $0  # Wyświetl linię po modyfikacji
    for (i = 1; i <= NF; i++) {
	if ($i == "{") {
    	    curly++
	}  else if ($i == "[") {
    	    square++
	} 
	}


}

