/*
 tools for exploring objects
*/

/* is the text x a path? return it if true or undefined otherwise */
function isRefString(x) {
        return typeof x === "string" &&  /^[$#]\//.test(x) ? x : undefined;
}

/* is the x a path reference? return the path string if true or undefined otherwise */
function isRef(x) {
        return x ? (isRefString(x) || isRefString(x['$ref'])) : undefined;
}

/* return the array representing the path */
function path2array(x) {
        if (x instanceof Array)
                return x;
        var p = isRef(x);
        return p ? p.substr(2).split('/') : undefined;
}

function array2path(x) {
        return (x instanceof Array) ? "#/" + x.join('/') : isRef(x);
}

function ONode(node, name, parent) {
        this.node = node;
        this.name = name;
        this.parent = parent;
        this.children = null;
}

ONode.prototype = {
        /* retrieve the root ONode */
        root: function() {
                return this.parent ? this.parent.root() : this;
        },

        /* is current node a child of the other? */
        isChildOf: function(other) {
                return this === other || (this.parent && this.parent.isChildOf(other));
        },

        /*
        * expands to reference the children's ONode
        * rec if defined is either the depth to expand (1 or lesser means
        * only this node, 2 means this node and its children, ...) or true
        * to expand indefinitely
        */
        expand: function(rec) {
                var i;
                var n;
                var c = this.children;
                if (!c) {
                        n = this.node;
                        c = {};
                        if (n instanceof Array) {
                                for(i = 0 ; i < n.length ; i++)
                                        c[i] = new ONode(n[i], i, this);
                        } else if (n instanceof Object) {
                                for(i in n)
                                        c[i] = new ONode(n[i], i, this);
                        }
                        this.children = c;
                }
                if (rec === true || Number(rec) > 1) {
                        var rerec = rec === true ? true : (Number(rec) - 1);
                        for(i in c)
                                c[i].expand(rerec);
                }
        },

        /*
        * the opposite of expand except that if rec is not false all children
        * are collapsed recursively
        */
        collapse: function(rec) {
                var c = this.children;
                if (c) {
                        this.children = null;
                        if (rec)
                                for(var i in c)
                                        c[i].collapse(true);
                }
        },

        /*
         * iteration over the expanded nodes
         * fun is called for each node
         */
        forEach: function(fun, rec) {
                var z = fun(this);
                if (z !== "stop") {
                        var u = undefined;
                        if (z !== "continue" && (rec === undefined || rec === true || Number(rec) > 1)) {
                                var rerec = rec === undefined || rec === true ? true : (Number(rec) - 1);
                                u = this.forEachChild(fun, rerec);
                        }
                        z = u;
                }
                return z;
        },

        forEachChild: function(fun, rec) {
                var u = undefined;
                var c = this.children;
                var rerec = rec === true ? true : (Number(rec) - 1);
                for(var i in c) {
                        u = c[i].forEach(fun, rerec);
                        if (u === "stop")
                                break;
                }
                return u;
        },

        /*
         * get the node of path given by array starting from current
         * node and from index 'i' of array 'arr'
         */
        at: function(arr, i) {
                var j = i ? i : 0;
                if (j == arr.length)
                        return this;
                if (j > arr.length)
                        return undefined;
                this.expand(false);
                if (arr[j] in this.children)
                        return this.children[arr[j]].at(arr, j+1);
                if (this.anchor && this.anchor.node)
                        return this.anchor.node.at(arr, j);
                return undefined;
        },

        nodeAt: function(arr, i) {
                var n = this.at(arr, i);
                return n ? n.node : n;
        },

        /*
         * get arrayed path of the current node
         */
        path: function() {
                if (!this.parent)
                        return [];
                var x = this.parent.path();
                x.push(this.name);
                return x;
        },

        /*
         * get the string path of current node
         */
        pathStr: function() {
                return array2path(this.path());
        }
};

function ODoc(root) {
        /* record the target and expand nodes from root */
        this.target = root;
        this.root = new ONode(root, "#", null);
        this.root.expand(true);

        /* collect links and anchors */
        var links = [];
        var anchors = {};
        this.links = links;
        this.anchors = anchors;
        this.root.forEach(function(n){
                var r = isRef(n.node);
                if (r) {
                        links.push(n);
                        var a = (r in anchors) ? anchors[r] : (anchors[r] = { links: [], name: r });
                        a.links.push(n);
                        n.isref = true;
                        n.anchor = a;
                        return "continue"; /* avoid having 2 refs for { "$ref": "#/x" } */
                }
        });

        /* check the required anchors */
        for(let i in anchors) {
                let an = anchors[i];
                let r = an.name;
                let path = path2array(r);
                an.path = path;
                let node = this.root.at(path);
                an.node = node;
                if (!node)
                        console.log("anchor "+anchors[a].name+" does not exist");
        }

        /* detect recursive links */
        for(let i in anchors) {
                let an = anchors[i];
                let z = [];
                let y = [ an ];
                while (y.length) {
                        let r = y.pop();
                        if (r.node) {
                                for (let l of links) {
                                        if (l.isChildOf(r.node)) {
                                                let t = l.anchor;
                                                if (!z.includes(t)) {
                                                        z.push(t);
                                                        y.push(t);
                                                }
                                        }
                                }
                        }
                }
                an.recursive = z.includes(an);
                if (an.recursive) {
                        console.log("recursive "+an.name+":");
                        z.forEach(function(a){console.log("  -> "+a.name)});
                }
        }
}

ODoc.prototype = {
        at: function(path) {
                return this.root.at(path2array(path));
        },

        nodeAt: function(path) {
                return this.root.nodeAt(path2array(path));
        },

        forEachChild: function(path, fun, rec) {
                var n = this.at(path);
                return n ? n.forEachChild(fun, rec) : undefined;
        }
};

exports.ODoc = ODoc;
