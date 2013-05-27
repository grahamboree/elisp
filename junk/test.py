


def read_from(tokens):
    if len(tokens) == 0:
        raise SyntaxError('unexpected EOF while reading')
    token = tokens.pop(0)
    if '(' == token:
        L = []
        while tokens[0] != ')':
            print tokens
            L.append(read_from(tokens))
        tokens.pop(0) # pop off ')'
        #print "list"
        #return L
    elif ')' == token:
        raise SyntaxError('unexpected )')
    else:
		print "atom " + str(token)
		#print token
        #return atom(token)

if __name__ == "__main__":
	tokens = ["(", "+", "(", "-", "1", "3", ")", "2", ")"]
	read_from(tokens)
