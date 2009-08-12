# Modul zum Einlesen von VCD-Dateien
from re import compile
from types import FloatType

class VCDError(Exception): pass

class VCDInternalError(VCDError): pass

class VCDParserError(VCDError):
  
  def __init__(self, parser, msg):
    self.__vcdname = parser._filename
    self.__lineno = parser._lineno
    self.__msg = msg
    
  def __str__(self):
    return self.__vcdname + ":" + str(self.__lineno) + ": " + self.__msg
  
class VCDResult(object): pass

class VCDTime(object):
  
  def __init__(self, value, conv):
    self.__time = value
    self.__conv = conv
    self.__edges = list()
    
  def add(self, edge):
    self.__edges.append(edge)
    
  @property
  def internalTime(self): return self.__time
  
class VCDEdge(object):
  
  def __init__(self, var, time, value, isInit):
    self.__init = isInit
    self.__value = value
    self.__time = time
    self.__var = var
    self.__size = var.size
    self.__intern = time.internalTime
    
  def bit(self, bitnum, width = 1):
    if bitnum >= self.__size:
      raise VCDInternalError, "invalid bit number, max is %d" % self.__size - 1
    if (bitnum + width) > self.__size:
      raise VCDInternalError, "invalid width, bitnum + width exeeded size, max is %d" % self.__size
    l = list(self.__value)
    l.reverse()
    l = l[bitnum:bitnum + width]
    l.reverse()
    return "".join(l)
    
  @property
  def value(self): return self.__value
  
  @property
  def intValue(self):
    try:
      return int(self.__value, 2)
    except ValueError:
      raise VCDError, "can't convert value to integer"
      
  @property
  def hasUnknown(self): return "x" in self.__value
  
  @property
  def valueUnknown(self): return ("x" * self.__size) == self.__value
  
  @property
  def hasTristate(self): return "z" in self.__value
  
  @property
  def valueTristate(self): return ("z" * self.__size) == self.__value
  
  @property
  def internalTime(self): return self.__intern
  
  @property
  def time(self): return self.__time
  
  @property
  def variable(self): return self.__variable
  
  @property
  def isInit(self): return self.__init
  
  def analyseWire(self, bitnum):
    def checkBit(e):
      if e.bit(bitnum) not in ("0", "1"):
        raise VCDError, "bit value isn't valid to analyse it"
    result = VCDResult()
    checkBit(self)
    c = self.__var.vcdInstance.conv2sec
    t0 = c(self.internalTime)
    e1 = self.__var.getNextEdge(self)
    checkBit(e1)
    t1 = c(e1.internalTime)
    e2 = self.__var.getNextEdge(e1)
    checkBit(e2)
    t2 = c(e2.internalTime)
    result.edges = [self, e1, e2]
    result.period = t2 - t0
    result.frequency = 1 / result.period
    result.pattern = self.bit(bitnum) + e1.bit(bitnum) + e2.bit(bitnum)
    if self.bit(bitnum) == "0":
      result.hightime = t2 - t1
      result.lowtime = t1 - t0
    else:
      result.hightime = t1 - t0
      result.lowtime = t2 - t1
    result.dutty = result.hightime / result.period
    return result
    
class VCDVar(object):
  
  def __init__(self, vcd, typ, size, name):
    self.__vcd = vcd
    self.__type = typ
    self.__size = size
    self.__name = name
    self.__edges = list()
    
  def add(self, edge):
    self.__edges.append(edge)
    
  @property
  def size(self): return self.__size
  
  @property
  def name(self): return self.__name
  
  @property
  def edgecount(self): return len(self.__edges) - 1
  
  @property
  def vcdInstance(self): return self.__vcd
  
  @property
  def initedge(self):
    if len(self.__edges) < 1:
      raise VCDError, "no init edge available"
    return self.__edges[0]
  
  @property
  def firstedge(self):
    if len(self.__edges) < 2:
      raise VCDError, "no edges available"
    return self.__edges[1]
  
  @property
  def lastedge(self):
    if len(self.__edges) < 2:
      raise VCDError, "no edges available"
    return self.__edges[-1]
  
  def getEdges(self, starttime = None, endtime = None):
    if not starttime == None and type(starttime) == FloatType:
      starttime = self.__vcd.conv2intern(starttime)
    if not endtime == None and type(endtime) == FloatType:
      endtime = self.__vcd.conv2intern(endtime)
    for e in self.__edges:
      if starttime is not None and e.internalTime < starttime: continue
      if endtime is not None and e.internalTime > endtime: break
      yield e
    
  def getPrevEdge(self, timeOrEdge):
    if isinstance(timeOrEdge, VCDEdge):
      # edge-Mode
      idx = self.__edges.index(timeOrEdge)
      if idx < 2:
        raise VCDError, "edge hasn't previous edge"
      return self.__edges[idx - 1]
    elif type(timeOrEdge) == FloatType:
      time = self.__vcd.conv2intern(timeOrEdge)
    else:
      time = timeOrEdge
    e0 = self.__edges[0]
    if e0.internalTime > time:
      raise VCDError, "no previous edge found, because start time is higher"
    for e in self.__edges[1:]:
      if e.internalTime >= time:
        return e0
      e0 = e
    return self.__edges[-1]
    
  def getNextEdge(self, timeOrEdge):
    if isinstance(timeOrEdge, VCDEdge):
      # edge-Mode
      idx = self.__edges.index(timeOrEdge)
      if idx >= (len(self.__edges) - 1):
        raise VCDError, "edge hasn't next edge"
      return self.__edges[idx + 1]
    elif type(timeOrEdge) == FloatType:
      time = self.__vcd.conv2intern(timeOrEdge)
    else:
      time = timeOrEdge
    for e in self.__edges[:-1]:
      if e.internalTime >= time:
        return e
    raise VCDError, "no next edge found, because end time to low"
  
class VCD(object):
  
  __rx_timescale = compile(r"^(\d+)\s*([pnum]?)s$")
  __timeunitmap = {
    "p": 0.000000000001,
    "n": 0.000000001,
    "u": 0.000001,
    "m": 0.001,
    "":  1.0,
  }
  __rx_edge = compile(r"^(([01zx])(\S)|b([01zx]+)\s(\S+))$")
  
  def __init__(self, filename):
    self.__time = None
    self.__starttime = None
    self.__timescale = None
    self.__namemap = dict()
    self.__scope = None
    self.__is_enddef = False
    self.__conv = None
    self.__timemap = list()
    self.__edgecount = 0
    self.__lineno = None
    self.__filename = None
    self.readByFilename(filename)
  
  def readByFilename(self, filename):
    self.__filename = filename
    try:
      stream = open(filename, "r")
    except IOError, e:
      raise VCDError, str(e)
    in_definition = False
    line = ""
    self.__lineno = 0
    for raw in stream.readlines():
      self.__lineno += 1
      if not raw.strip(): continue
      #print "[%s]" % raw
      if in_definition:
        # suche nach "$end"
        line += raw
        if raw.find("$end") >= 0:
          self.__read_def(line.strip())
          line = ""
          in_definition = False
      else:
        if raw.lstrip()[0] == "$":
          if raw.find("$end") >= 0:
            # einzeilige Def.
            self.__read_def(raw.strip())
          else:
            in_definition = True
            line = raw
        elif raw[0] == "#":
          try:
            t = long(raw[1:].strip())
          except ValueError:
            raise VCDParserError(self, "can't read timeslot, wrong time format")
          if self.__time == None:
            self.__starttime = t
          self.__time = VCDTime(t, self.__conv)
          self.__timemap.append(self.__time)
        else:
          self.__parse_edge(raw.strip())
    stream.close()
  
  def __read_def(self, defstring):
    defstring = defstring[1:-4].strip() # erstes "$" und letztes "$end" entfernen
    l = defstring.split(None, 1)
    if len(l) == 1:
      keyword, tail = l[0], None
    else:
      keyword, tail = l
    # bearbeiten der Keywords
    if keyword == "enddefinitions":
      if self.__scope:
        raise VCDParserError(self, "unresolved scope")
      self.__is_enddef = True
    elif keyword == "upscope":
      l = self.__scope.split(".")
      if len(l) > 1:
        self.__scope = ".".join(l[-1])
      else:
        self.__scope = None
    elif keyword == "scope":
      typ, name = tail.split()
      if self.__scope:
        self.__scope += "." + name
      else:
        self.__scope = name
    elif keyword == "timescale":
      self.__parse_timescale(tail)
    elif keyword == "var":
      if not self.__scope:
        raise VCDParserError(self, "var definition not into scope definition")
      self.__parse_var(tail)
    elif keyword == "version":
      pass # skip definition
    elif keyword == "date":
      pass # skip definition
    elif keyword == "dumpvars":
      if self.__starttime == None:
        raise VCDParserError(self, "dumpvars is defined before starttime")
      tail = tail.replace("\r\n", "\n")
      for value in tail.split("\n"):
        self.__parse_edge(value, True)
    else:
      raise VCDParserError(self, "unknown definition: %s = '%s'" % (keyword, tail))
    
  def __parse_timescale(self, value):
    m = self.__rx_timescale.match(value)
    if not m:
      raise VCDParserError(self, "parser error on timescale setting: '%s'" % value)
    n, u = m.groups()
    self.__conv = self.__timeunitmap[u] * long(n)
    
  def __parse_var(self, value):
    l = value.split()
    typ, size, vid, name = l[:4]
    size = int(size)
    self.__namemap[vid] = VCDVar(self, typ, size, self.__scope + "." + name)
    
  def __parse_edge(self, value, isDump = False):
    m = self.__rx_edge.match(value.strip())
    if not m:
      raise VCDParserError(self, "format error in change line: '%s'" % value)
    l = m.groups()
    if l[1] == None:
      v, i = l[3:5]
    else:
      v, i = l[1:3]
    if not i in self.__namemap:
      raise VCDParserError(self, "format error in change line: '%s': id not found" % value)
    var = self.__namemap[i]
    if not var.size == len(v):
      raise VCDParserError(self, "format error in change line: '%s': size dosn't match, expected=%d, found=%d" % (value, var.size, len(v)))
    try:
      e = VCDEdge(var, self.__time, v, isDump)
    except VCDInternalError, e:
      raise VCDParserError(self, str(e))
    self.__time.add(e)
    var.add(e)
    if not isDump: self.__edgecount += 1
    
  @property
  def starttime(self): return self.__starttime
  
  @property
  def endtime(self): return self.__time.internalTime
  
  @property
  def edgecount(self): return self.__edgecount
  
  @property
  def timecount(self): return len(self.__timemap) - 1
  
  @property
  def _filename(self): return self.__filename or "<unknown>"
  
  @property
  def _lineno(self): return self.__lineno or -1
  
  @property
  def conv2sec(self):
    def _t2s(itime):
      return itime * self.__conv
    return _t2s
    
  @property
  def conv2intern(self):
    def _s2t(stime):
      return long(stime / self.__conv)
    return _s2t
    
  @property
  def variables(self):
    result = self.__namemap.values()
    result.sort(None, lambda x: x.name)
    return result
    
  def getVariable(self, name):
    for v in self.__namemap.values():
      if v.name == name: return v
    raise VCDError, "variable '%s' not found" % name
    
# EOF
