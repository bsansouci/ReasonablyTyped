open Parser_flow;

let combine_programs =
  List.fold_left
    (
      fun (current_id, all_code) program => {
        let typeof_table =
          switch program {
          | Modulegen.BsDecl.ModuleDecl _ statements => Typetable.create statements
          | _ => []
          };
        let program = Optimizer.optimize typeof_table program;
        switch (Codegen.program_to_code program typeof_table) {
        | Some (program_id, program_code) when program_id !== "" => (
            program_id,
            all_code ^ "\n" ^ program_code
          )
        | Some (program_id, program_code) => (current_id, all_code ^ "\n" ^ program_code)
        | None => (current_id, all_code)
        }
      }
    )
    ("Unknown ID", "");

let compile module_name module_def => {
  let (statements, errors) = {
    let (ocaml_ast, errors) =
      Parser_flow.program_file module_def (Some (Loc.SourceFile module_name));
    let (_, statements, _) = ocaml_ast;
    (statements, errors)
  };
  let programs = List.map Modulegen.statement_to_program statements;
  let flow_code = String.concat "\n" (List.map Flowprinter.show_decl programs);
  let (module_id, bs_code) = combine_programs programs;
  (module_id, flow_code, bs_code)
};