### $ANTLR 2.7.7 (2006-11-01): "yacc.g" -> "YaccLexer.py"$
### import antlr and other modules ..
import sys
import antlr

version = sys.version.split()[0]
if version < '2.2.1':
    False = 0
if version < '2.3':
    True = not False
### header action >>> 

### header action <<< 
### preamble action >>> 

### preamble action <<< 
### >>>The Literals<<<
literals = {}


### import antlr.Token 
from antlr import Token
### >>>The Known Token Types <<<
SKIP                = antlr.SKIP
INVALID_TYPE        = antlr.INVALID_TYPE
EOF_TYPE            = antlr.EOF_TYPE
EOF                 = antlr.EOF
NULL_TREE_LOOKAHEAD = antlr.NULL_TREE_LOOKAHEAD
MIN_USER_TYPE       = antlr.MIN_USER_TYPE
ID = 4
COLON = 5
SEMICOLON = 6
CHAR = 7
STRING = 8
ERROR = 9
PREC = 10
DPREC = 11
UINT = 12
ACTION = 13
OR = 14
HYPHEN = 15
CARROT = 16
BANG = 17
LETTER = 18
DIGIT = 19
SL_COMMENT = 20
ML_COMMENT = 21
WS = 22

class Lexer(antlr.CharScanner) :
    ### user action >>>
    ### user action <<<
    def __init__(self, *argv, **kwargs) :
        antlr.CharScanner.__init__(self, *argv, **kwargs)
        self.caseSensitiveLiterals = True
        self.setCaseSensitive(True)
        self.literals = literals
    
    def nextToken(self):
        while True:
            try: ### try again ..
                while True:
                    _token = None
                    _ttype = INVALID_TYPE
                    self.resetText()
                    try: ## for char stream error handling
                        try: ##for lexical error handling
                            la1 = self.LA(1)
                            if False:
                                pass
                            elif la1 and la1 in u':':
                                pass
                                self.mCOLON(True)
                                theRetToken = self._returnToken
                            elif la1 and la1 in u';':
                                pass
                                self.mSEMICOLON(True)
                                theRetToken = self._returnToken
                            elif la1 and la1 in u'-':
                                pass
                                self.mHYPHEN(True)
                                theRetToken = self._returnToken
                            elif la1 and la1 in u'^':
                                pass
                                self.mCARROT(True)
                                theRetToken = self._returnToken
                            elif la1 and la1 in u'!':
                                pass
                                self.mBANG(True)
                                theRetToken = self._returnToken
                            elif la1 and la1 in u'|':
                                pass
                                self.mOR(True)
                                theRetToken = self._returnToken
                            elif la1 and la1 in u'0123456789':
                                pass
                                self.mUINT(True)
                                theRetToken = self._returnToken
                            elif la1 and la1 in u'\'':
                                pass
                                self.mCHAR(True)
                                theRetToken = self._returnToken
                            elif la1 and la1 in u'"':
                                pass
                                self.mSTRING(True)
                                theRetToken = self._returnToken
                            elif la1 and la1 in u'{':
                                pass
                                self.mACTION(True)
                                theRetToken = self._returnToken
                            elif la1 and la1 in u'\t\n\r ':
                                pass
                                self.mWS(True)
                                theRetToken = self._returnToken
                            else:
                                if (self.LA(1)==u'e') and (self.LA(2)==u'r') and (self.LA(3)==u'r') and (self.LA(4)==u'o') and (self.LA(5)==u'r') and (True) and (True):
                                    pass
                                    self.mERROR(True)
                                    theRetToken = self._returnToken
                                elif (self.LA(1)==u'%') and (self.LA(2)==u'p'):
                                    pass
                                    self.mPREC(True)
                                    theRetToken = self._returnToken
                                elif (self.LA(1)==u'%') and (self.LA(2)==u'd'):
                                    pass
                                    self.mDPREC(True)
                                    theRetToken = self._returnToken
                                elif (self.LA(1)==u'/') and (self.LA(2)==u'/'):
                                    pass
                                    self.mSL_COMMENT(True)
                                    theRetToken = self._returnToken
                                elif (self.LA(1)==u'/') and (self.LA(2)==u'*'):
                                    pass
                                    self.mML_COMMENT(True)
                                    theRetToken = self._returnToken
                                elif (_tokenSet_0.member(self.LA(1))) and (True) and (True) and (True) and (True) and (True) and (True):
                                    pass
                                    self.mID(True)
                                    theRetToken = self._returnToken
                                else:
                                    self.default(self.LA(1))
                                
                            if not self._returnToken:
                                raise antlr.TryAgain ### found SKIP token
                            ### option { testLiterals=true } 
                            self.testForLiteral(self._returnToken)
                            ### return token to caller
                            return self._returnToken
                        ### handle lexical errors ....
                        except antlr.RecognitionException, e:
                            raise antlr.TokenStreamRecognitionException(e)
                    ### handle char stream errors ...
                    except antlr.CharStreamException,cse:
                        if isinstance(cse, antlr.CharStreamIOException):
                            raise antlr.TokenStreamIOException(cse.io)
                        else:
                            raise antlr.TokenStreamException(str(cse))
            except antlr.TryAgain:
                pass
        
    def mCOLON(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = COLON
        _saveIndex = 0
        pass
        self.match(':')
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mSEMICOLON(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = SEMICOLON
        _saveIndex = 0
        pass
        self.match(';')
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mHYPHEN(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = HYPHEN
        _saveIndex = 0
        pass
        self.match('-')
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mCARROT(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = CARROT
        _saveIndex = 0
        pass
        self.match('^')
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mBANG(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = BANG
        _saveIndex = 0
        pass
        self.match('!')
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mOR(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = OR
        _saveIndex = 0
        pass
        self.match('|')
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mPREC(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = PREC
        _saveIndex = 0
        pass
        self.match("%prec")
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mDPREC(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = DPREC
        _saveIndex = 0
        pass
        self.match("%dprec")
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mERROR(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = ERROR
        _saveIndex = 0
        pass
        self.match("error")
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mID(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = ID
        _saveIndex = 0
        pass
        self.mLETTER(False)
        while True:
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in u'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz':
                pass
                self.mLETTER(False)
            elif la1 and la1 in u'0123456789':
                pass
                self.mDIGIT(False)
            elif la1 and la1 in u'_':
                pass
                self.match('_')
            elif la1 and la1 in u'.':
                pass
                self.match('.')
            else:
                    break
                
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mLETTER(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = LETTER
        _saveIndex = 0
        pass
        la1 = self.LA(1)
        if False:
            pass
        elif la1 and la1 in u'abcdefghijklmnopqrstuvwxyz':
            pass
            self.matchRange(u'a', u'z')
        elif la1 and la1 in u'ABCDEFGHIJKLMNOPQRSTUVWXYZ':
            pass
            self.matchRange(u'A', u'Z')
        else:
                self.raise_NoViableAlt(self.LA(1))
            
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mDIGIT(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = DIGIT
        _saveIndex = 0
        pass
        pass
        self.matchRange(u'0', u'9')
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mUINT(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = UINT
        _saveIndex = 0
        pass
        _cnt33= 0
        while True:
            if ((self.LA(1) >= u'0' and self.LA(1) <= u'9')):
                pass
                self.mDIGIT(False)
            else:
                break
            
            _cnt33 += 1
        if _cnt33 < 1:
            self.raise_NoViableAlt(self.LA(1))
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mCHAR(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = CHAR
        _saveIndex = 0
        pass
        self.match('\'')
        la1 = self.LA(1)
        if False:
            pass
        elif la1 and la1 in u'\\':
            pass
            pass
            self.match('\\')
            self.matchNot(antlr.EOF_CHAR)
        elif la1 and la1 in u'\u0000\u0001\u0002\u0003\u0004\u0005\u0006\u0007\u0008\t\n\u000b\u000c\r\u000e\u000f\u0010\u0011\u0012\u0013\u0014\u0015\u0016\u0017\u0018\u0019\u001a\u001b\u001c\u001d\u001e\u001f !"#$%&()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[]^_`abcdefghijklmnopqrstuvwxyz{|}~\u007f':
            pass
            pass
            self.match(_tokenSet_1)
        else:
                self.raise_NoViableAlt(self.LA(1))
            
        self.match('\'')
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mSTRING(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = STRING
        _saveIndex = 0
        pass
        self.match('"')
        while True:
            if (_tokenSet_2.member(self.LA(1))):
                pass
                self.matchNot('"')
            else:
                break
            
        self.match('"')
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mACTION(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = ACTION
        _saveIndex = 0
        pass
        self.match('{')
        lcount= 1
        incomment= False
        indquote= False
        insquote= False
        while lcount != 0:
           if self.LA(1) == '\\':
               self.consume()
           elif self.LA(1) == '/' and self.LA(2) == '*':
               if not indquote and not insquote:
                   incomment= True
               self.consume()
           elif self.LA(1) == '*' and self.LA(2) == '/':
               if not indquote and not insquote:
                   incomment= False
               self.consume()
           elif self.LA(1) == '\'':
               if not indquote and not incomment:
                   insquote= not insquote
           elif self.LA(1) == '"':
               if not insquote and not incomment:
                   indquote= not indquote
           elif self.LA(1) == antlr.EOF:
               _ttype = antlr.EOF
           elif self.LA(1) == '\n':
               self.newline()
           elif not indquote and not insquote and not incomment:
               if self.LA(1)== '}':
                   lcount -= 1
               elif self.LA(1)== '{':
                   lcount += 1
           self.consume()
        _ttype = antlr.SKIP;
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mSL_COMMENT(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = SL_COMMENT
        _saveIndex = 0
        pass
        self.match("//")
        while True:
            if (_tokenSet_3.member(self.LA(1))):
                pass
                self.match(_tokenSet_3)
            else:
                break
            
        la1 = self.LA(1)
        if False:
            pass
        elif la1 and la1 in u'\n':
            pass
            self.match('\n')
        elif la1 and la1 in u'\r':
            pass
            self.match('\r')
            if (self.LA(1)==u'\n'):
                pass
                self.match('\n')
            else: ## <m4>
                    pass
                
        else:
            ##<m3> <closing
                pass
            
        _ttype = antlr.SKIP; self.newline();
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mML_COMMENT(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = ML_COMMENT
        _saveIndex = 0
        pass
        self.match("/*")
        while True:
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in u'\n':
                pass
                self.match('\n')
                self.newline();
            elif la1 and la1 in u'\u0000\u0001\u0002\u0003\u0004\u0005\u0006\u0007\u0008\t\u000b\u000c\u000e\u000f\u0010\u0011\u0012\u0013\u0014\u0015\u0016\u0017\u0018\u0019\u001a\u001b\u001c\u001d\u001e\u001f !"#$%&\'()+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\u007f':
                pass
                self.match(_tokenSet_4)
            else:
                if (self.LA(1)==u'\r') and (self.LA(2)==u'\n') and ((self.LA(3) >= u'\u0000' and self.LA(3) <= u'\u007f')) and ((self.LA(4) >= u'\u0000' and self.LA(4) <= u'\u007f')) and (True) and (True) and (True):
                    pass
                    self.match('\r')
                    self.match('\n')
                    self.newline();
                elif ((self.LA(1)==u'*') and ((self.LA(2) >= u'\u0000' and self.LA(2) <= u'\u007f')) and ((self.LA(3) >= u'\u0000' and self.LA(3) <= u'\u007f')) and ( self.LA(2)!='/' )):
                    pass
                    self.match('*')
                elif (self.LA(1)==u'\r') and ((self.LA(2) >= u'\u0000' and self.LA(2) <= u'\u007f')) and ((self.LA(3) >= u'\u0000' and self.LA(3) <= u'\u007f')) and (True) and (True) and (True) and (True):
                    pass
                    self.match('\r')
                    self.newline();
                else:
                    break
                
        self.match("*/")
        _ttype = antlr.SKIP;
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mWS(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = WS
        _saveIndex = 0
        pass
        _cnt59= 0
        while True:
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in u' ':
                pass
                self.match(' ')
            elif la1 and la1 in u'\r':
                pass
                self.match('\r')
                self.match('\n')
                self.newline()
            elif la1 and la1 in u'\n':
                pass
                self.match('\n')
                self.newline()
            elif la1 and la1 in u'\t':
                pass
                self.match('\t')
            else:
                    break
                
            _cnt59 += 1
        if _cnt59 < 1:
            self.raise_NoViableAlt(self.LA(1))
        _ttype = antlr.SKIP
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    

### generate bit set
def mk_tokenSet_0(): 
    ### var1
    data = [ 0L, 576460743847706622L, 0L, 0L]
    return data
_tokenSet_0 = antlr.BitSet(mk_tokenSet_0())

### generate bit set
def mk_tokenSet_1(): 
    ### var1
    data = [ -549755813889L, -268435457L, 0L, 0L]
    return data
_tokenSet_1 = antlr.BitSet(mk_tokenSet_1())

### generate bit set
def mk_tokenSet_2(): 
    ### var1
    data = [ -17179869185L, -1L, 0L, 0L]
    return data
_tokenSet_2 = antlr.BitSet(mk_tokenSet_2())

### generate bit set
def mk_tokenSet_3(): 
    ### var1
    data = [ -9217L, -1L, 0L, 0L]
    return data
_tokenSet_3 = antlr.BitSet(mk_tokenSet_3())

### generate bit set
def mk_tokenSet_4(): 
    ### var1
    data = [ -4398046520321L, -1L, 0L, 0L]
    return data
_tokenSet_4 = antlr.BitSet(mk_tokenSet_4())
    
### __main__ header action >>> 
if __name__ == '__main__' :
    import sys
    import antlr
    import YaccLexer
    
    ### create lexer - shall read from stdin
    try:
        for token in YaccLexer.Lexer():
            print token
            
    except antlr.TokenStreamException, e:
        print "error: exception caught while lexing: ", e
### __main__ header action <<< 
