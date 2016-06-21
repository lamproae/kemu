package main

import (
	//	"errors"
	"fmt"
	"github.com/ajstarks/svgo"
	"html/template"
	"io"
	"io/ioutil"
	"net/http"
	"os"
	"regexp"
	"strings"
)

var tmpl = `<!DOCTYPE html>
<html lang="en">
<head>
	<meta charset="UTF-8">
	<title>Func list</title>
</head>

<body>
{{range .Funcs}}
<p><a href={{.}}>{{.}}</a> </p>
	{{end}}
</body>
</html>
`

var FetchRootName = regexp.MustCompile(`Tracing[[:space:]]+\"(?P<func>[[:word:]_]+)\".*end\.`)
var FetchFunctionName = regexp.MustCompile(`(?P<func>[[:word:]_\.]+\(\))`)
var RemoveCComment = regexp.MustCompile(`(?P<cc>\/\\*.*\*\/)`)
var RemoveCPPComment = regexp.MustCompile(`(?P<cc>\/\/.*)`)

var FetchFunctionCall = regexp.MustCompile(`(?P<func>[[:word:]_\.]+\(\))`)
var RemoveFileHead = FetchRootName
var RemoveFileTail = regexp.MustCompile(`(?P<fe>Ending.*)`)
var RemoveUselessInformation = regexp.MustCompile(`(?P<useless>0[[:space:][:word:]\.\!\\+#)]*\|)`)

type CFunc struct {
	name    string
	body    string
	calling []*CFunc
}

type CFuncs struct {
	Funcs []string
}
type CFuncCallTree struct {
	root *CFunc
	list map[string]*CFunc
}

/*
    x,y        w ump
	----------------------
        |                    |
       h|                    |
        |                    |
	----------------------
	         dmp
*/
type CFuncGraph struct {
	x, y                   int
	tx, ty                 int
	w, h                   int /* width, height */
	umpx, umpy, dmpx, dmpy int /* up middler port, down middler point */
	name                   string
	level                  int
}

type Graph struct {
	gw, gh         int /* graph size */
	ew, eh         int /* element size */
	sw, sh         int /* Start point */
	deltaw, deltah int /* delta value between to elements */
	pix            int /* pixls */
	rows           map[int]*RowGraph
	graphs         map[string]*CFuncGraph
}

type RowGraph struct {
	x, y    int
	element []*CFuncGraph
}

type CallTreeSPF struct {
	root   string
	vertex map[string]*SPFVertex
	rows   map[int][]*SPFVertex
	level  int
}

type SPFVertex struct {
	fn       *CFunc
	level    int
	drawed   bool
	children []*SPFVertex
}

var constGraph = Graph{gw: 4000, gh: 4000, ew: 0, eh: 15, sw: 1000, sh: 40, deltaw: 20, deltah: 40, pix: 5}

var CTreeRoot = CFuncCallTree{}

func (spf *CallTreeSPF) getSubVertex(v *CFunc) {
	if v == nil || len(v.calling) == 0 {
		return
	}

	spf.vertex[v.name].children = make([]*SPFVertex, 0, len(v.calling))
	for _, fi := range v.calling {
		if _, ok := spf.vertex[fi.name]; ok {
			continue
		}

		if fi != nil {
			nv := new(SPFVertex)
			nv.fn = fi
			nv.level = spf.vertex[v.name].level + 1
			if spf.level < nv.level {
				spf.level = nv.level
			}
			nv.drawed = false
			spf.vertex[fi.name] = nv
			spf.vertex[v.name].children = append(spf.vertex[v.name].children, nv)
			spf.getSubVertex(fi)
		}
	}
}

func (spf *CallTreeSPF) getVertex() error {
	if r, ok := CTreeRoot.list[spf.root]; ok {
		spf.vertex = make(map[string]*SPFVertex, 200)
		nv := new(SPFVertex)
		nv.fn = r
		nv.level = 0
		nv.drawed = false
		if spf.level < nv.level {
			spf.level = nv.level
		}
		spf.vertex[spf.root] = nv
		if len(r.calling) != 0 {
			spf.getSubVertex(r)
		}
	}

	spf.rows = make(map[int][]*SPFVertex, spf.level)
	spf.rows[0] = append(spf.rows[0], spf.vertex[spf.root])
	spf.vertex[spf.root].drawed = true
	for l := 1; l <= spf.level; l++ {
		for _, p := range spf.rows[l-1] {
			for _, c := range p.children {
				if spf.vertex[c.fn.name].drawed == true {
					continue
				}
				spf.rows[l] = append(spf.rows[l], c)
				spf.vertex[c.fn.name].drawed = true
			}
		}
	}
	/*
		for _, v := range spf.vertex {
			spf.rows[v.level] = append(spf.rows[v.level], v)
		}
	*/
	return nil
}

func dumpFuncList(w http.ResponseWriter, req *http.Request) {
	w.Header().Set("Content-Type", "image/svg+xml")
	s := svg.New(w)
	h := 15
	sw := 500
	sh := 500
	s.Start(100000, 100000)
	for v, _ := range CTreeRoot.list {
		s.Roundrect(sw, sh, len(v)*5, h, 1, 1, "fill:none;stroke:black")
		s.Text((sw+len(v)*5)-(len(v)*5)/2, sh+10, v, "text-anchor:middle;font-size:5px;fill:black")

		sw += len(v)*5 + 20
	}
	s.End()
}

func showFuncList(w http.ResponseWriter, req *http.Request) {
	req.ParseForm()
	fmt.Println("Path: ", req.URL.Path)
	var funcList = CFuncs{Funcs: make([]string, 0, len(CTreeRoot.list))}

	if req.URL.Path == "/" {
		for fn, fi := range CTreeRoot.list {
			if len(fi.calling) == 0 {
				continue
			}
			funcList.Funcs = append(funcList.Funcs, fn)
		}
		t, err := template.New("main").Parse(tmpl)
		if err != nil {
			fmt.Println(err.Error())
			return
		}
		t.Execute(w, funcList)

		return
	}

	createFuncGraph(w, req)
}

func createFuncGraph(w http.ResponseWriter, r *http.Request) {
	fmt.Println(r.URL.Path[1:])
	if fn, ok := CTreeRoot.list[r.URL.Path[1:]]; ok {
		spf := new(CallTreeSPF)
		spf.root = fn.name
		spf.level = 0
		spf.getVertex()
		buildFuncCallTreeGraph(spf)
		drawFuncCallTree(w, spf)
	} else {
		io.WriteString(w, "No exit in global list\n")
	}
}

func buildRowGraph(level int, row []*SPFVertex) *RowGraph {
	rg := new(RowGraph)
	rlen := 0
	for _, v := range row {
		rlen = rlen + len(v.fn.name)*constGraph.pix + constGraph.deltaw
	}

	rg.x = constGraph.sw - rlen/2
	rg.y = constGraph.sh + (constGraph.eh+constGraph.deltah)*level
	rg.element = make([]*CFuncGraph, 0, len(row))
	sw := rg.x
	sh := rg.y
	for _, v := range row {
		var graph = new(CFuncGraph)
		graph.name = v.fn.name
		graph.x = sw
		graph.y = sh
		graph.w = len(graph.name) * constGraph.pix
		graph.h = constGraph.eh
		graph.tx = graph.x + len(graph.name)*constGraph.pix - (len(graph.name)*constGraph.pix)/2
		graph.ty = graph.y + constGraph.eh - constGraph.pix
		graph.umpx = graph.x + graph.w/2
		graph.umpy = graph.y
		graph.dmpx = graph.x + graph.w/2
		graph.dmpy = graph.y + constGraph.eh
		graph.level = level
		rg.element = append(rg.element, graph)
		constGraph.graphs[graph.name] = graph
		sw += graph.w + constGraph.deltaw
	}

	return rg
}

func buildFuncCallTreeGraph(spf *CallTreeSPF) {
	constGraph.rows = make(map[int]*RowGraph, spf.level)
	constGraph.graphs = make(map[string]*CFuncGraph, len(spf.vertex))
	for i, row := range spf.rows {
		constGraph.rows[i] = buildRowGraph(i, row)
	}
}

func drawFuncCallTree(w http.ResponseWriter, spf *CallTreeSPF) {
	if spf == nil {
		return
	}
	w.Header().Set("Content-Type", "image/svg+xml")
	s := svg.New(w)
	s.Start(constGraph.gw, constGraph.gh)
	for _, row := range constGraph.rows {
		for _, g := range row.element {
			s.Roundrect(g.x, g.y, g.w, g.h, 1, 1, "fill:none;stroke:black")
			s.Link(g.name, g.name)
			s.Text(g.tx, g.ty, g.name, "text-anchor:middle;font-size:5px;fill:black")
			s.LinkEnd()
		}
	}

	for _, v := range spf.vertex {
		for _, subv := range v.children {
			s.Line(constGraph.graphs[v.fn.name].dmpx, constGraph.graphs[v.fn.name].dmpy, constGraph.graphs[subv.fn.name].umpx, constGraph.graphs[subv.fn.name].umpy, "fill:none;stroke:black")
		}
	}
	s.End()
}

func getFuncBody(buf string, figure string) string {
	index := strings.Index(buf, figure)
	body := buf[index+len(figure):]
	var brace_count = 0
	var bsize = 0
	var bstart = 0

	for _, c := range body {
		bsize++
		if bsize == 1 && c != '{' {
			break
		}
		if c == '{' {
			if brace_count == 0 {
				bstart = 1
			}
			brace_count++
		} else if c == '}' {
			brace_count--
			if brace_count == 0 {
				break
			}
		}
	}

	if bsize == 1 {
		body = ""
	} else {
		body = body[bstart : bstart+bsize-2]
	}
	return body
}

func buildCallTree() {
	for fn, fi := range CTreeRoot.list {
		if fi.body == "" {
			CTreeRoot.list[fn].calling = nil
			continue
		}

		CTreeRoot.list[fn].calling = make([]*CFunc, 0, 10)
		fmt.Println(fn + " Body: " + fi.body)
		current := 0
		fstart := current
		insub := 0
		bcount := 0
		for _, c := range fi.body {
			current++
			if c == ')' && insub == 0 {
				fn_name := fi.body[fstart : current-2]
				//				fmt.Println(fn_name)
				CTreeRoot.list[fn].calling = append(CTreeRoot.list[fn].calling, CTreeRoot.list[fn_name])

				if fi.body[current] == '{' {
					insub = 1
					//					bcount++
				}
				continue
			}

			if c == ';' && insub == 0 {
				fstart = current
				continue
			}

			if c == '{' {
				bcount++
				if insub == 0 {
					insub = 1
				}
				continue
			}

			if c == '}' {
				bcount--
				if bcount == 0 {
					insub = 0
					fstart = current
				}
			}
		}
	}
}

func main() {

	if len(os.Args) != 2 {
		fmt.Println("Please input file name")
		os.Exit(-1)
	}

	file, err := os.Open(os.Args[1])
	defer file.Close()
	if err != nil {
		os.Exit(-1)
	}

	buf, _ := ioutil.ReadAll(file)
	//fmt.Println(string(buf))
	//fmt.Println(strings.TrimRight(string(buf), " "))
	s := string(buf)
	if FetchRootName.MatchString(s) {
		root := new(CFunc)
		//fmt.Println(root.name)
		root.name = FetchRootName.FindStringSubmatch(s)[1][:len(FetchFunctionName.FindStringSubmatch(s)[1])-2]
		root.body = getFuncBody(s, root.name+"()")
		//	fmt.Println(root.name)
		CTreeRoot.root = root
		//fmt.Println(CTreeRoot)

	}
	if RemoveUselessInformation.MatchString(s) {
		s = RemoveUselessInformation.ReplaceAllString(s, "")
		//	fmt.Println(s)
	}

	s = RemoveCComment.ReplaceAllString(s, "")
	s = RemoveCPPComment.ReplaceAllString(s, "")
	s = RemoveFileHead.ReplaceAllString(s, "")
	s = RemoveFileTail.ReplaceAllString(s, "")
	s = strings.Replace(s, " ", "", -1)
	s = strings.Replace(s, "\t", "", -1)
	line := strings.Split(s, "\n")
	var count = 0
	for _, l := range line {
		if FetchFunctionName.MatchString(l) {
			count = count + 1
		}
		//fmt.Println(l)
	}

	s = strings.Replace(s, "\n", "", -1)
	//fmt.Println(s)
	CTreeRoot.list = make(map[string]*CFunc, count)
	CTreeRoot.list[CTreeRoot.root.name] = CTreeRoot.root
	for _, l := range line {
		if FetchFunctionName.MatchString(l) {
			if _, ok := CTreeRoot.list[FetchFunctionName.FindStringSubmatch(l)[1]]; ok {
				continue
			}
			new_fn := new(CFunc)
			new_fn.name = FetchFunctionName.FindStringSubmatch(l)[1][:len(FetchFunctionName.FindStringSubmatch(l)[1])-2]
			new_fn.body = getFuncBody(s, new_fn.name+"()")
			CTreeRoot.list[new_fn.name] = new_fn
		}
	}

	buildCallTree()
	for fn, fi := range CTreeRoot.list {
		fmt.Println(fn)
		fmt.Println("----------------------------------")
		for _, f := range fi.calling {
			fmt.Println("+" + f.name)
		}
	}

	http.Handle("/", http.HandlerFunc(showFuncList))
	http.Handle("/circle", http.HandlerFunc(dumpFuncList))
	err = http.ListenAndServe(":2003", nil)
	if err != nil {
		fmt.Println(err.Error())
	}
}
