## Reflection-related: requires extending generator
- [x] Test with (multiple) inheritance
- [x] Test multiple input files
- [x] Add appropriate error handling for de-serialization
- [x] Add reflector based on Boost.Hana
- [ ] Add another generator to prove expandability: maybe for getting members by name in general, similar to one of the proposals
- [x] Add documentation (install instructions, usage)
- [x] Allow making 3rdparty classes/structs reflectable
    - [x] Add additional parameter for code generator to allow specifying relevant classes
          explicitely
    - [x] Fix traits currently relying on `JsonSerializable` being base class
- [x] Allow exporting symbols
- [x] Fix the massive number of warnings which are currently being created by the code generator (missing `-resource-dir` was the problem)
- [ ] Test with libc++ (currently only tested with libstdc++)
- [ ] Support templated classes
- [ ] Allow (de)serialization of static members (if that makes sense?)
- [ ] Allow ignoring particular members or selecting specificly which member variables should be considered
    * This could work similar to Qt's Signals & Slots macros.
    * but there should also be a way to do this for 3rdparty types.
    * Note that currently, *all* public member variables are (de)serialized.
- [ ] Allow using getter and setter methods
    * [ ] Allow to serialize the result of methods.
    * [ ] Allow to pass a deserialized value to a method.
* [ ] Validate enum values when deserializing
    * Likely undoable with Boost.Hana
* [ ] Untie serialization and deserialization

## Library-only
- [x] Support `std::unique_ptr` and `std::shared_ptr`
- [x] Support `std::map` and `std::unordered_map`
- [ ] Support `std::any`
- [ ] Support `std::optional`
- [x] Support/document customized (de)serialization (eg. serialize some `DateTime` object to ISO string representation)
