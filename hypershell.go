package devtools

import (
	"bytes"
	"fmt"
	"os"
	"os/exec"
	"sort"
	"sync"
)

var currentShell string

func init() {
	currentShell = os.Getenv("SHELL")
}

type SubShell struct {
	dir string
}

func NewSubShell(path string) (*SubShell, error) {
	file, err := os.Open(path)
	if err != nil {
		return nil, fmt.Errorf("Failed to open file while creating a sub shell: %w", err)
	}
	defer file.Close()

	info, err := file.Stat()
	if err != nil {
		return nil, fmt.Errorf("Failed to read file stats while creating a sub shell: %w", err)
	}

	if !info.IsDir() {
		return nil, fmt.Errorf("Failed to create a sub shell: '%s' is not a directory", path)
	}

	shell := SubShell{
		dir: path,
	}

	return &shell, nil
}

func (s *SubShell) Run(command string) ShellResult {
	buffer := bytes.Buffer{}

	cmd := exec.Command(currentShell, "-c", command)
	cmd.Dir = s.dir
	cmd.Stdout = &buffer
	cmd.Stderr = &buffer

	result := ShellResult{Dir: s.dir}
	result.Error = cmd.Run()
	result.Result = buffer.String()

	return result
}

type ShellResult struct {
	Dir    string
	Result string
	Error  error
}

type HyperShell struct {
	subShells []*SubShell
}

func NewHyperShell() *HyperShell {
	return &HyperShell{[]*SubShell{}}
}

func (s *HyperShell) Append(subShell *SubShell) {
	s.subShells = append(s.subShells, subShell)
}

func (s *HyperShell) Run(command string) []ShellResult {
	results := []ShellResult{}
	resultChan := make(chan ShellResult, len(s.subShells))
	wg := sync.WaitGroup{}

	for _, subShell := range s.subShells {
		wg.Add(1)
		go func(s *SubShell) {
			resultChan <- s.Run(command)
			wg.Done()
		}(subShell)
	}
	wg.Wait()
	close(resultChan)

	for result := range resultChan {
		results = append(results, result)
	}

	sort.Slice(results, func(i, j int) bool {
		return results[i].Dir < results[j].Dir
	})

	return results
}
