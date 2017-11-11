## Reflection-related: requires extending generator
- [x] Test with (multiple) inheritance
- [x] Test multiple input files
- [x] Add appropriate error handling for de-serialization
- [x] Add reflector based on Boost.Hana
- [ ] Add another generator to prove expandability: maybe for getting members by name in general, similar to one of the proposals
- [x] Add documentation (install instructions, usage)
- [ ] Fix the massive number of warnings which are currently being created
- [ ] Test with libc++ (currently only tested with libstdc++)
- [ ] Support enums (undoable with Boost.Hana)
- [ ] Support templated classes
- [x] Allow making 3rdparty classes/structs reflectable
    - [x] Add additional parameter for code generator to allow specifying relevant classes
          explicitely
    - [x] Fix traits currently relying on `JsonSerializable` being base class
- [x] Allow exporting symbols

## Library-only
- [ ] Support `std::unique_ptr` and `std::shared_ptr`
- [ ] Support `std::map` and `std::unordered_map`
- [ ] Support `std::any`
- [x] Support/document customized (de)serialization (eg. serialize some `DateTime` object to ISO string representation)
