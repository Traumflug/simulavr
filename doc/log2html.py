# Script to create html report from delivery-check script output
import sys
import os
import ConfigParser
from xml.dom.minidom import getDOMImplementation
import datetime
import shutil

class XMLPage(object):

  def __init__(self, cfg, filename, title):
    self.cfg = cfg
    self.__filename = filename
    self.__title = title
    impl = getDOMImplementation()
    self.__doc = impl.createDocument(None, "html", None)
    self.__root = self.__doc.documentElement
    self.__header()
    self.body = self.add(self.__root, "body")

  def write(self):
    p = self.cfg.get("global", "output")
    d = open(os.path.join(p, self.__filename), "w")
    self.__doc.writexml(d, encoding = "UTF-8")
    d.close()

  def __header(self):
    h = self.add(self.__root, "head")
    self.add(h, "title", text = self.__title)
    self.add(h, "link", { "rel": "stylesheet", "href": "report.css", "type": "text/css" })
    self.add(h, "meta", { "http-equiv": "Content-Type", "content": "text/html; charset=utf-8" })
    
  def add(self, root, name, attr = dict(), text = None):
    d = self.__doc.createElement(name)
    root.appendChild(d)
    for k, v in attr.items(): d.setAttribute(k, v)
    if text is not None:
      t = self.__doc.createTextNode(text)
      d.appendChild(t)
    return d

  def addText(self, root, text):
    t = self.__doc.createTextNode(text)
    root.appendChild(t)

  def getStartTime(self, section):
    return datetime.datetime(*map(int, self.cfg.get(section, "start").split("/")))
    
  def getEndTime(self, section):
    return datetime.datetime(*map(int, self.cfg.get(section, "end").split("/")))
    
  def getTimeDelta(self, section):
    return self.getEndTime(section) - self.getStartTime(section)

  def addOverviewRow(self, root, header, data):
    r = self.add(root, "tr")
    self.add(r, "td", { "class": "tbl-header" }, header)
    self.add(r, "td", { "class": "tbl-data" }, data)
      
  def getFlag(self, name, cfgname = None):
    if cfgname is not None:
      if self.cfg.get("global", cfgname) == "n": return "no"    
    if self.cfg.get("global", name) == "y": return "yes"
    return "no"

  def addTable(self, root, title):
    t = self.add(root, "table", { "class": "tbl-overview"})
    self.add(t, "caption", text = title)
    return t

  def addCommitRow(self, root):
    r = self.add(root, "tr")
    self.add(r, "td", { "class": "tbl-header" }, "HEAD commit")
    d = self.add(r, "td", { "class": "tbl-data" })
    t = open(self.cfg.get("global", "commit-repo"), "r").readlines()
    c_id = t[0].split()[1]
    c_date = t[2].split(":", 1)[1].strip()
    c_subject = t[4].strip()
    self.add(d, "div", text = c_id)
    self.add(d, "div", text = "on " + c_date)
    self.add(d, "div", text = c_subject)

  def addRepoStatusRow(self, root):
    r = self.add(root, "tr")
    self.add(r, "td", { "class": "tbl-header" }, "repo status after build check")
    d = self.add(r, "td", { "class": "tbl-data" })
    txt = open(self.cfg.get("global", "status-repo"), "r").read().strip()
    if len(txt) > 0:
      for t in txt.split("\n"):
        self.add(d, "div", text = t.strip())
    else:
      self.add(d, "div", text = "<clean>")

  def addLogRow(self, root, section):
    r = self.add(root, "tr")
    d = self.add(r, "td", { "class": "tbl-data" })
    self.add(d, "a", { "href": section + ".html" }, section)
    self.add(r, "td", { "class": "tbl-data" }, str(self.getStartTime(section)))
    self.add(r, "td", { "class": "tbl-data right-align" }, str(self.getTimeDelta(section)))
    res = "-"
    if self.cfg.has_option(section, "result"): res = self.cfg.get(section, "result")
    self.add(r, "td", { "class": "tbl-data right-align" }, res)

  def addVersions(self, root, title):
    if not self.cfg.has_section("versions"): return
    if not self.cfg.has_option("versions", "tools"): return
    t = self.addTable(root, title)
    for item in self.cfg.get("versions", "tools").split():
      if not self.cfg.has_option("versions", item) or not self.cfg.has_option("versions", item + "-text"):
        continue
      ver = self.cfg.get("versions", item)
      txt = self.cfg.get("versions", item + "-text")
      self.addOverviewRow(t, txt, ver)

def createIndex(cfg):
  p = XMLPage(cfg, "index.html", "Simulavr build check report")
  p.add(p.body, "h1", text = "Simulavr build check report")
  d = p.add(p.body, "div", { "class": "result" }, "Simulavr build check was:")
  if cfg.get("global", "result") == "success":
    p.add(d, "span", attr = { "class": "result-success" }, text = "successfull!")
  else:
    p.add(d, "span", attr = { "class": "result-failed" }, text = "not successfull!")

  t = p.addTable(p.body, "Overview")
  p.addOverviewRow(t, "build check started on", str(p.getStartTime("global")))
  p.addOverviewRow(t, "overall time for build check", str(p.getTimeDelta("global")))

  t = p.addTable(p.body, "Build options")
  p.addOverviewRow(t, "with dist check", p.getFlag("opt-dist"))
  p.addOverviewRow(t, "with python / python module", p.getFlag("opt-python"))
  p.addOverviewRow(t, "with tcl / tcl module", p.getFlag("opt-tcl", "conf-tcl"))
  p.addOverviewRow(t, "with api documentation", p.getFlag("opt-doxy", "conf-doxy"))
  p.addOverviewRow(t, "with manual / web pages", p.getFlag("opt-sphinx"))
  p.addOverviewRow(t, "with verilog module", p.getFlag("opt-verilog", "conf-verilog"))

  t = p.addTable(p.body, "Repository")
  p.addOverviewRow(t, "URL", p.cfg.get("global", "repo"))
  p.addOverviewRow(t, "branch on repository", p.cfg.get("global", "branch"))
  p.addCommitRow(t)
  p.addRepoStatusRow(t)

  p.addVersions(p.body, "Used tool versions")
  
  p.add(p.body, "a", { "name": "logs" })
  t = p.addTable(p.body, "Log files")
  r = p.add(t, "tr")
  p.add(r, "td", { "class": "tbl-header" }, "build step")
  p.add(r, "td", { "class": "tbl-header" }, "start date")
  p.add(r, "td", { "class": "tbl-header" }, "build time")
  p.add(r, "td", { "class": "tbl-header" }, "return code")
  for s in p.cfg.get("global", "steps").strip().split(): p.addLogRow(t, s)

  p.write()

def copyUtilityFiles(cfg):
  o_path = cfg.get("global", "output")
  r_path = cfg.get("global", "repo-work")
  src = os.path.join(r_path, "doc", "report.css")
  shutil.copy(src, o_path)
  src = os.path.join(r_path, "doc", "formatcode.js")
  shutil.copy(src, o_path)

def createLogPage(cfg, section):
  p = XMLPage(cfg, section + ".html", "Build step: " + section)
  p.add(p.body, "h2", text = "Build step: " + section)
  d = p.add(p.body, "div", { "class": "backlink" })
  p.add(d, "a", { "href": "index.html#logs" }, "Back to overview")

  p.add(p.body, "div", { "class": "emphasized" }, "Command line:")
  p.add(p.body, "div", { "class": "preformat" }, cfg.get(section, "cmd"))

  p.add(p.body, "div", { "class": "emphasized" }, "Build step runtime:")
  p.add(p.body, "div", { "class": "preformat" }, str(p.getTimeDelta(section)))

  t = open(cfg.get(section, "log"), "r").read().strip()
  l = "text"
  if section == "configure": l = "configure"
  if section in ("make", "check", "dist"): l = "make"
  if section.startswith("install"): l = "make"
  p.add(p.body, "div", { "class": "emphasized" }, "Log:")
  p.add(p.body, "pre", { "lang": l, "class": "code" }, t)
  p.add(p.body, "script", { "type": "text/javascript", "src": "formatcode.js" }, "")

  p.write()

if __name__ == "__main__":

  if not len(sys.argv) is 2:
    print >>sys.stderr, "error: parameter config file expected"
    sys.exit(1)
  if not os.path.exists(sys.argv[1]):
    print >>sys.stderr, "error: config file '%s' not found" % sys.argv[1]
    sys.exit(2)
  cfg = ConfigParser.ConfigParser()
  try:
    cfg.read(sys.argv[1])
  except Exception:
    print >>sys.stderr, "error: read config file failed"
    sys.exit(1)

  copyUtilityFiles(cfg)
  createIndex(cfg)
  for s in cfg.get("global", "steps").strip().split(): createLogPage(cfg, s)

  sys.exit(0)

# EOF
