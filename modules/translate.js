'use strict';

const getString = require('./getString.js');
const getObject = require('./getObject.js');

const lookup = require('./lookup.js');

const { AST } = require('./ast.js');

const translateLib = require('./translateLib.js')(translate);



function translate(exp, callexp) {
  if (typeof exp === 'string') return exp;

  // TODO: consider using typeof exp != 'object'
  if (!exp || !exp.type ) return '?';

  let self = translate;

  switch (exp.type) {

    // Literals return the textual definiton of the literal
    case 'Literal':
      return JSON.stringify(exp.value);

    // Look up identifiers
    case 'Identifier': // TODO: we'll need to handle scope (path)
      return lookup(exp);

    // Template literals
    case 'TemplateLiteral':
      // TODO: proper conversion of template literals with embedded expressions
      return JSON.stringify(getString(exp));

    // Member expressions are usually translated to built-in methods
    case 'MemberExpression':
      const obj = self(exp.object);
      //const deepObj = AST.getMemberExpressionDeepObjectId();

      // Computed expression, could be array access
      // TODO: this only caters for the initializer-type global array
      // definitions, and needs to be folded into transforms/array
      if (exp.computed) {
        // Check in globals, TODO: find variable in-scope
        let glob = self.game.globals[obj]

        // Some kind of Array on the global scope -> render as element access
        if (glob && glob.type.substr(-2) == '[]') {
          return obj+'[' +self(exp.property)+ ']'
        }
      }

      // MicroCanvas calls
      //if (obj === self.game.alias || obj.match(/^(g|s)fx/)){ // Game asset properties (gfx & sfx)
      //  return translateLib(exp, callexp);
      //
      //} else if (deepObj && deepObj.match(/^(g|s)fx/)) { // Game asset properties (gfx & sfx)
      //  return translateLib(exp, callexp);

      if (true) {
        return translateLib(exp, callexp);
      // Some other library
      } else {
        // Simple property access
        //if (exp.property.type === 'Identifier') {
        //  return '? ' + (self(exp.object) + '.' + self(exp.property));
        //} this shouldn't be needed now

        // Property expression
        return '(' + (self(exp.object) + ').' + self(exp.property));
        // TODO: property access belongs to subexpressions, move it there
        // TODO: add one-level-deep ternary support
        // TODO: support bracket notation for complex properties
        // TODO: do not parenthetise simple identifiers
      }

    // Define a new variable on the current scope
    case 'VariableDeclaration':
      return exp.declarations.map(dec => self(dec)).join(' ');

    case 'VariableDeclarator':
      let id = getString(exp.id),
          initializer = exp.init ? self(exp.init) : undefined;

      let v = self.game.createVariable(id, initializer, undefined, exp);

      return (
        (v.type ? v.type : self.game.guessType(v.id, v.value, v.$scope)) + ' '
        + v.cid
        + (typeof v.value !== 'undefined' ? ' = ' + v.value : '')
        + ';'
      );

    // Just unwrap expression body
    case 'ExpressionStatement':
      return self(exp.expression) +';';

    // And wrap a block statement
    case 'BlockStatement':
      return '{\n'+exp.body.map(e => '  '+self(e)).join('\n') +'\n}';

    // Loops
    case 'WhileStatement':
      return 'while ('+self(exp.test)+') ' + self(exp.body);

    // For loop
    case 'ForStatement':
      // init / test / update / body
      let loopInit = self(exp.init).replace(/;$/,'');

      return 'for ('+loopInit+'; '+self(exp.test)+'; '+self(exp.update)+') '+self(exp.body);
      break;

    // If statements are all the same
    case 'IfStatement':
      return 'if ('+self(exp.test)+') '
        +( exp.consequent
           ? self(exp.consequent) + ( exp.alternate ? ' else ' + self(exp.alternate) : '')
           : ''
        );

    // Similarly, conditionals too
    case 'ConditionalExpression':
      return '('+self(exp.test)
        +( exp.consequent
           ? ' ? '+self(exp.consequent) + ( exp.alternate ? ' : '+self(exp.alternate) : '')
           : ''
        )+')'; // TODO: smarter parens

    // Function calls
    case 'CallExpression':
      // TODO: this is the culprit for the need for Identifier checks in translateLib
      // e.g. this can send anyFunctionCall(42) to translateLib (and it shouldn't),
      // it should handle plain function calls' lookup in place
      return translateLib(exp.callee, exp);

    // Return statements
    case 'ReturnStatement':
      return 'return' + (exp.argument ? ' '+self(exp.argument) : '') + ';';

    // Break statement
    case 'BreakStatement':
      return 'break;';


    // Assignment and binary expressions work pretty much unchanged across JS/C
    case 'LogicalExpression': // TODO: parens?
    case 'BinaryExpression': // TODO: parens!
    case 'AssignmentExpression':
      let op = exp.operator

      // Handle triple-equals
      if (op === '===') op = '==';

      // Special handling required for some game.* objects
      if (exp.type == 'AssignmentExpression'
        && exp.left.type == "MemberExpression"
      ) {
        const r = translateLib(exp);

        // Only finish early if we have received a transform result
        if (r !== undefined) return r;
      }

      let parens = false;

      // TODO: http://en.cppreference.com/w/c/language/operator_precedence

      // Add parentesis for binary +/- operators
      if (exp.type === 'BinaryExpression' && (op === '+' || op === '-')) {
        if (callexp // only embedded expressions need parens
            && callexp.operator !== '=' // no parens needed on assignment RHS
        ) {
          parens = true;
        }
      }

      // Add parentesis for bitwise <</>> operators
      if (exp.type === 'BinaryExpression' && (op === '<<' || op === '>>')) {
        parens = true;
      }

      // Add parentesis for compund assignment operators /=,*=
      if (exp.type === 'AssignmentExpression' && (op === '/=' || op === '*=')) {
        parens = true;
      }

      return (
        (parens ? '(' : '')
        + self(exp.left, exp)
        + ' ' + op + ' '
        + self(exp.right, exp)
        + (parens?')':'')
      );

    // Unary expression (pre + postfix) work mostly the same
    // TODO: boolean tricks? (!something >>> 1-something)
    case 'UpdateExpression':
    case 'UnaryExpression':
      return (exp.prefix
        ? exp.operator + self(exp.argument)
        : self(exp.argument) + exp.operator
      );

    // Yield has a special meaning in microcanvas
    // (acts as a terminator of a generator-function frame)
    case 'YieldExpression':
      translate.game.generators = true; // enable feature
      return '_microcanvas_yield('+getString(exp.argument)+')';
  }

  return '__translate("'+exp.type+'", "'+(exp.$raw||'')+'")';
}


// Translates all function call arguments (or optionally, array
// initializer elements) and returns the result, enclosed in
// the correct brackets.
function translateArgs(args, array = false) {
  return (array ? '[' : '(')
    +(args.length
      ? ' '+args.map(arg => translate(arg)).join(', ')+' '
      : ''
    )
  +(array ? ']' : ')');

}



translate.args = translateArgs;
translate.arrs = (args) => translateArgs(args, true);
module.exports = translate;
