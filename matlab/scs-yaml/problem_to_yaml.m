function problem_to_yaml(destination, problem_name, data, K)
%PROBLEM_TO_YAML serializes SuperSCS problem data to YAML
%
%Syntax:
%problem_to_yaml(destination, problem_name, data, K)
%
%Input arguments:
% destination       a file path (a string; either a relative or an absolute
%                   file path) OR the output of fopen(path, 'w')
% problem_name      a name for your problem 
% data              a structure with the problem data; the structure should
%                   contain the fields: `A`, `b` and `c`
% K                 the cones
%
%Examples:
% problem_to_yaml('my_problem.yml', 'problem-1', data, K)
%   :: this will save the problem data contained in `data` and `K` in the 
%   file `my_problem.yml` in the current directory
%
% problem_to_yaml(1, 'problem-1', data, K)
%   :: this will print the data to the standard output stream
%
% problem_to_yaml(2, 'problem-1', data, K)
%   :: this will print the data to the standard error stream
%
% fid = fopen('path/to/myfile.yml', 'w');
% problem_to_yaml(fid, 'problem-1', data, K)
% fclose(fid);
%  :: this
%
%See also
%sparse_to_csc

if ~isfield(K,'f');K.f = 0;end
if ~isfield(K,'l');K.l = 0;end
if ~isfield(K,'q');K.q = [];end
if ~isfield(K,'s');K.s = [];end
if ~isfield(K,'ep');K.ep = 0;end
if ~isfield(K,'ed');K.ed = 0;end
if ~isfield(K,'p');K.p = [];end

should_close_fid = 0;
if ischar(destination),
    fid = fopen(destination, 'w');
    should_close_fid = 1;
elseif isnumeric(destination)
    fid = destination;    
end
space = '    ';
fprintf(fid, '--- # SuperSCS Problem\n');
fprintf(fid, 'problem:\n');
fprintf(fid, '%sname: ''%s''\n', space, problem_name);
yamlify_sparse_matrix(fid, data.A)
fprintf(fid, '%sb: ', space);
yamlify_array(fid, data.b)
fprintf(fid, '%sc :', space);
yamlify_array(fid, data.c)
fprintf(fid, '%sK:\n', space);
fields_K = fieldnames(K);
for i=1:numel(fields_K)
    val = sprintf('%g',K.(fields_K{i}));
    if isempty(val), val = '[]'; end
    fprintf(fid, '%s%s%s: %s\n', space, space, fields_K{i}, val);    
end
fprintf(fid, '...');
if should_close_fid,
    fclose(fid);
end

function yamlify_sparse_matrix(fid, A)
space = '    ';
double_space = [space space];
[m, n, nnz, a, ir, jc] = sparse_to_csc(A);
fprintf(fid, '%sA: \n', space);
fprintf(fid, '%sm: %d\n', double_space, m);
fprintf(fid, '%sn: %d\n', double_space, n);
fprintf(fid, '%snnz: %d\n', double_space, nnz);
fprintf(fid, '%sa: ', double_space);
yamlify_array(fid, a);
fprintf(fid, '%sI: ', double_space);
yamlify_array(fid, ir);
fprintf(fid, '%sJ: ', double_space);
yamlify_array(fid, jc);


function yamlify_array(fid, x)
fprintf(fid, '[');
fprintf(fid, '%g,', x(1:end-1));
fprintf(fid, '%g]\n', x(end));