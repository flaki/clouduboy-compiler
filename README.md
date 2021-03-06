# The Clouduboy C compiler
> At the end of the day, C/C++ is pretty similar to JavaScript - maybet that's _enough_.

My thoughts were something along those lines when years ago I came up with the idea of the `clouduboy-compiler`. Back then, Clouduboy was simply a web-based code editor for the Clouduboy, and it used the exact same language one would use with the Arduino IDE: C++. Using JavaScript seemed to have obvious benefits - but I have seen "it's actually pretty similar!" lead people down the wrong path too many times (for example, at my friends at the Tessel Project's first incarnation). It took me some time to try, and even more to convince myself that this was something useful, worthwile - and more importantly, in the realm of possibility.


## Perks
- You write your game in JavaScript. I mean c'mon, it's wayyyy nicer than C++ will ever be.
- You write your game. You save it. You click a button, and you can play it right in the browser - no flashing, no nothing, easy testing and debugging with the browser's devtools.
- You don't need any fancy equipment, no even any of the microcontrollers. You could share your games to be played by others on laptops, tablets, phones,even!
- Done coding? Click another button and your code is compiled in the cloud and travels to your machine to be readily flashed, if you use the Clouduboy Flasher it's even easier.
- If you are not a coder, this might be a nice way for you to learn a thing or two about programming.
- There are actually many more of these, but... you get the idea.


## Caveats, a.k.a. compiler limitations
All right, so this is already too good to be true. Where's the catch?

Well, the catch is - Clouduboy cannot compile just _any_ JavaScript that uses Canvas... you will have to play by the rules. And there are quite a bit of those. The problem is, every single feature you want to use has to be re-implemented and than proper transforms should exist to convert code between those representations. It's not trivial to say the least. That said, it can be good enough, and features can be incrementally added.

Oh did we mention this will compile and run whatever you write to be run on an 8-bit, 16MHz microcontroller? Oh yeah, so whatever is the output going to be, it needs to be mindful of resources and heavily optimized.

Please note: this is not an exhaustive list of _all_ the limitations ever, but more like an FAQ to help you better navigate what's (currently) possible and what not.

Please also note: **nothing** is inherently impossible to do, given enough developer brainpower, but there are two fundamental constraints:

1. development time (this is the more restrictive) - I can only spend so much time working on Clouduboy (this is my little pet project, after all). I have an an unending pool of TODO, with which if you want to help feel free to (all of Clouduboy is open-source and the complete code is openly accessible on GitHub). That said, feel free to contribute in any way, but also don't _expect_ anything.
2. device flash (device & program memory) - even the most complex feature feature can be re-implemented and polyfilled in C/C++. The problem is: how much code will it generate? Some of the devices that Clouduboy targets have a few kilobytes of memory with 30-ish kilobytes of flash (program) space. This is barely enough for anything and any features that Clouduboy adds will take away precious space from your _own_ code and ideas.


### What do these labels mean?
* `warn` - the Clouduboy compiler will transform the code for you to C/C++ but your code may not compile and/or may not run, or crash, or eat your laundry. Usually the warnings that Clouduboy will show you also contain some info on how to avoid the warning in the first place. That said, there is a pretty good chance that in the browser your code/game will work just fine even if it generates warnings.
* `error` - crash & burn, baby! If you bump into one of these, the Clouduboy compiler will outright refuse to compile your code to C/C++ (and thus won't even attempt compiling it to a binaregendy HEX). Chances are that your code will run just fine in the browser, but unless you change it to comply with the stated limitations you won't be able to compile it and put it onto a precious pocketable plaything.


## Limitations

`Math.random`:

- `warn` Please consider using game.random for generating random integers.

`game.drawText`:

- `error` String concatenation/manipulation is currently unsupported. Please use template literals  (`${foo} something {bar}`).
- `warn` Long text does not wrap, and there is a limitation of 32 characters for the template literal + expression buffer (template  literals that contain no expressions compile to string literals and, thus, unaffected by this)
- Fonts are not currently supported (there is one built-in font that all text functions use) but making it possible to use a custom ASCII bitmap font in your game is being considered/explored, mainly because of textual and text-heavy games and because the current default font has a lots of compromises. You can always include your own limited fonts as sprites, though, keeping in mind that images take a lot of program memory in general.
- `warn` Please note that the legacy `{fill|center}Text(x,y,text)` parameter order is deprecated — please use ``…text,x,y)` instead.

`game.*` a.k.a. the Canvas:
- Most of the Canvas-to-C-support is mapped incrementally, on demand - if your favorite function/feature is missing file an issue on GitHub with a usecase - or even better, send a pull request :)
-

Sounds and music:

- There is an initial experimental chiptune library built in (that provides the chirps for, among other things, the Dino game), but it's highly experimental and mostly unmaintained at this point. Cross-device support for audio is especially tricky, due to all kinds of different hardware capabilities and interrupt settings.

Graphics and sprites:

- Sprites are provided by the PIF (Pixeldata Image Format) library, and have a bunch of limitations.
- `warn` No color support right now - but support being heavily explored with the Tiny Arcade as a flagship device (and the Gamebuino Meta that's coming soon).
- `error` Currently we need to know in compile time which image asset a `drawImage` call will be operating on (because we need to know the width/height for drawing). This means dynamically specifying the sprite (like returning it from a function) is not supported and results in a compiler error. Some level of dynamism can be achieved via ternaries and multi-frame sprites, or you could just use a good-ole-fashioned `if`/`switch` block.
- `game.frameCount` is incremented at the very begining of every every `loop()` iteration.
- Plese take care! `game.frameCount` compiles to an **`unsigned`** `int`. Any operations/calculations must take this into account — `game.frameCount - (game.frameCount+1) > 0` will be obviously true in JavaScript (`=-1`), but once compiled this will **explicitly not be true** (`=65535`, or similar, as the unsigned rolls over)!

`Ternaries`:

* `warn` In library calls (e.g. drawImage) ternaries are reused, if they contain side effects that might cause problems on execution.

`Array`-s:

* `error` Arrays can only be created on the global scope.
* `error` Array literals can only be used on the global scope in the initializer.
* `error` The Array initializer (yes, on the global scope), needs to inform the compiler about the size of the array, as currently all these arrays are static, pre-allocated arrays that cannot be resized.
* `error` Passing arrays as function parameters are supported (experimental!), but with plenty of caveats. The biggest is probably: due to JavaScript's dynamic nature, the compiler needs to learn about which properties are used for passing in arrays, so you will have to have a call somewhere in the code that does just that. As we said, it's fragile. But it works.
* `error` Functions returning arrays are not supported.
* `error` Arrays of sprites are currently unsupported. Please consider using name prefixes or multiframe sprites instead.
* `error` Currently only arrays of numbers (signed integers) are supported. Support for byte arrays using TypedArrays is planned.
* `error` Arrays of arrays (multi-dimensional arrays) are currently not supported (but are being considered--please file an issue on the GitHub repo with a usecase!). That said, accessing a frame of a sprite using a frame number stored in an array (e.g. `gfxFoo[myArray[3]]`) is __valid syntax__ and works as intended.

`Object`-s:

* `error` Objects are currently unsupported (but are being considered), in the meantime please use a prefixed globals instead.

`Function`-s:

- `error` Nested functions are currently not supported. Please use functions declared on the global scope instead.
- `warn` Please consider using function declarations instead of function-valued let/const bindings. Function object created via function expressions & arrow functions and stored as variables declared on the global scope are currently converted to function declarations and are automatically hoisted & available throughout the whole code (no dead-zone).

`Date`-s and other std library modules:

- `error` The `Date` object is not supported. To keep track of in-game time, try using the global `frameCount`, which is kinda like timestamps but with a much younger epoch. ;)
- `error` The `RegExp` object and regular expressions in general are not supported. Not sure what would be a usecase for them on these tiny machines, though.



# License

> This software is licensed under the Apache License, version 2 ("ALv2"), quoted below.
>
> Copyright 2018 István Szmozsánszky, Clouduboy <https://clouduboy.org/>
>
> Licensed under the Apache License, Version 2.0 (the "License");
> you may not use this file except in compliance with the License.
> You may obtain a copy of the License at
>
> http://www.apache.org/licenses/LICENSE-2.0
>
> Unless required by applicable law or agreed to in writing, software
> distributed under the License is distributed on an "AS IS" BASIS,
> WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
> See the License for the specific language governing permissions and
> limitations under the License.
