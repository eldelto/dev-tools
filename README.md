
# Table of Contents

1.  [Dev Tools](#org481d202)
    1.  [hyper-shell](#org2f82e72)
    2.  [hyper-git](#org0b06afc)
        1.  [sync](#orga8f6cf8)
        2.  [Update](#orgb9864c8)


<a id="org481d202"></a>

# Dev Tools

Small convenience tools useful for dealing with work.


<a id="org2f82e72"></a>

## hyper-shell

**hyper-shell [directories]**

Multiplexes shell commands to multiple directories.


<a id="org0b06afc"></a>

## hyper-git

**hyper-git [-c config-path] [command]**

Executes git commands for all configured repositories. See the
*examples* folder for a config file to get started.


<a id="orga8f6cf8"></a>

### sync

Executes git clone, stash, checkout <default branch> & pull.


<a id="orgb9864c8"></a>

### Update

-   if project does not exist `git clone`
-   `git stash`
-   `git checkout <default-branch>`
-   `git pull`

