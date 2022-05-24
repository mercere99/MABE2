/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021.
 *
 *  @file  Genome.hpp
 *  @brief Base genome representation for organisms.
 */

#ifndef MABE_GENOME_HPP
#define MABE_GENOME_HPP

#include <limits>

#include "emp/base/error.hpp"
#include "emp/math/Random.hpp"
#include "emp/meta/TypeID.hpp"

namespace mabe {

  // Interface class for all genome types.
  class Genome {
  public:
    // Note: No constructors given; Genomes are pure virtual and as such must always   
    //       be constructed via a derived class.

    virtual emp::Ptr<Genome> Clone() = 0;         // Make an exact copy of this genome.
    virtual emp::Ptr<Genome> CloneProtocol() = 0; // Copy everything in this genome except sequence.

    virtual size_t GetSize() const = 0;           // Return the size of this genome using underlying type.
    virtual void Resize(size_t new_size) = 0;     // Set new size using underlying type.
    virtual void Resize(size_t new_size, double default_val) = 0;
    virtual size_t GetNumBytes() const = 0;       // Return number of bytes of data in this genome.
    virtual void SetSizeRange(size_t min_size, size_t max_size) = 0; // Put limits on genome size.

    virtual bool IsValid(size_t pos) const { return pos < GetSize(); }
    virtual size_t ValidatePosition(size_t pos) const { return pos; } // If circular, should mod, etc.

    virtual void Randomize(emp::Random &, size_t /*pos*/) = 0; // Randomize only at one locus

    // Randomize whole genome
    virtual void Randomize(emp::Random & random) {
      for (size_t i = 0; i < GetSize(); i++) Randomize(random, i); 
    }
    // Test for mutations in whole genome; return number of mutations occurred.
    virtual size_t Mutate(emp::Random & random) = 0;

    // Human-readable (if not easily understandable) shorthand representations.
    virtual std::string ToString() const { return "[unknown]"; }
    virtual void FromString(std::string & in) { emp_error("Cannot read genome from string."); }

    // Potentially more compressed or structured formats for saving/loading genomes.
    // TODO:
    // Archive & Serialize(Archive & archive) = 0;

    // Genome accessors for individual values…
    virtual int ReadInt(size_t index) const = 0;
    virtual double ReadDouble(size_t index) const = 0;
    virtual std::byte ReadByte(size_t index) const = 0;
    virtual bool ReadBit(size_t index) const = 0;

    // NEED MULTI-READ AND MULTI-WRITE.
    // NEED READS AND WRITES THAT ARE RESTRICTED TO A RANGE
    // EXAMPLE:
    virtual int ReadInt(size_t index, int min, int max) const = 0;

    virtual void WriteInt(size_t index, int value) =  0;
    virtual void WriteDouble(size_t index, double value) =  0;
    virtual void WriteByte(size_t index, std::byte value) =  0;
    virtual void WriteBit(size_t index, bool value) =  0;

    // Genome accessors for multiple values…
    // TODO:
    //  emp::span<int> ReadInts(size_t start_index, size_t end_index) =  0;
    //  emp::span<double> ReadDoubles(size_t start_index, size_t end_index) =  0;
    //  emp::span<std::byte> ReadBytes(size_t start_index, size_t end_index) =  0;
    //  emp::BitVector ReadBits(size_t start_index, size_t end_index) =  0;
    //
    // + Multi-writes!!

    class Head {
    protected:
      emp::Ptr<Genome> genome;  // Attached genome.
      size_t pos;               // What position is this head located at?
      int direction = 1;        // Direction forward is forward.

      static const constexpr unsigned int NORMAL=0;
      static const constexpr unsigned int END_OF_GENOME=1;
      static const constexpr unsigned int END_OF_CHROMOSOME=2;
      static const constexpr unsigned int INVALID=4;

      unsigned int state = NORMAL;

    public:
      Head(Genome & in_genome, size_t in_pos=0, int in_dir=1)
        : genome(&in_genome), pos(in_pos), direction(in_dir) { }
      Head(const Head &) = default;
      Head & operator=(const Head &) = default;

      // Check for Heads at the same position and same direction!
      bool operator==(const Head & in) const {
          return genome==in.genome && pos==in.pos && direction==in.direction;
      }
      bool operator!=(const Head & in) const { return !(*this == in); }

      bool IsValid() const { return genome->IsValid(pos); }

      // @CAO VALIDATIONS SHOULD HAPPEN BEFORE ACTIONS, NOT AFTER.

      Head & SetPosition(size_t in_pos) {
        pos = genome->ValidatePosition(in_pos);
        if (!IsValid()) state = INVALID;
        return *this;
      }

      Head & Advance(size_t factor=1) { return SetPosition(pos + direction * factor); }

      int ReadInt() { int out = IsValid() ? genome->ReadInt(pos) : 0; Advance(); return out; }
      double ReadDouble() { double out = IsValid() ? genome->ReadDouble(pos) : 0; Advance(); return out; }
      std::byte ReadByte() { std::byte out = IsValid() ? genome->ReadByte(pos) : std::byte(0); Advance(); return out; }
      bool ReadBit() { bool out = IsValid() ? genome->ReadBit(pos) : 0; Advance(); return out; }

      Head & WriteInt(int value) { if (IsValid()) genome->WriteInt(pos, value); return Advance(); }
      Head & WriteDouble(double value) { if (IsValid()) genome->WriteDouble(pos, value); return Advance(); }
      Head & WriteByte(std::byte value) { if (IsValid()) genome->WriteByte(pos, value); return Advance(); }
      Head & WriteBit(bool value) { if (IsValid()) genome->WriteBit(pos, value); return Advance(); }

      // NEED MULTI-READ AND MULTI-WRITE.
      // NEED READS AND WRITES THAT ARE RESTRICTED TO A RANGE
      // EXAMPLE OF RANGED-READ:
      int ReadInt(int min, int max) {
        int out = IsValid() ? genome->ReadInt(pos, min, max) : 0; Advance();
        return out;
      }

      void Reset() { pos = 0; state = NORMAL; direction = 1; }
      void ReverseDirection() { direction = -direction; }

      bool AtBegin() { return pos == 0; }
      bool AtEnd() { return pos == genome->GetSize(); }

      void Randomize(emp::Random & random) { genome->Randomize(random, pos); }
    };

    Head GetHead(size_t position=0, int direction=1) { return Head(*this, position, direction); }
    Head begin() { return GetHead(); }
    Head end() { return GetHead(GetSize()); }
    Head rbegin() { return GetHead(GetSize(), -1); }
    Head rend() { return GetHead(0, -1); }
  };


  template <typename LOCUS_T>
  class TypedGenome : public Genome {
  protected:
    using locus_t = LOCUS_T;
    using this_t = TypedGenome<LOCUS_T>;

    emp::vector<locus_t> data;                        // Actual data in the genome.
    double mut_p = 0.0;                               // Mutation probability (LOTS TO DO HERE!)
    size_t min_size = 0;
    size_t max_size = std::static_cast<size_t>(-1);

    double alphabet_size = 4.0;

  public:
    TypedGenome<LOCUS_T>() { }
    TypedGenome<LOCUS_T>(this_t &) = default;

    emp::Ptr<Genome> Clone() override { return emp::NewPtr<this_t>(*this); }
    emp::Ptr<Genome> CloneProtocol() override {
      emp::Ptr<Genome> out_ptr = NewPtr<this_t>();
      out_ptr->mut_p = mut_p;
      out_ptr->min_size = min_size;
      out_ptr->max_size = max_size;
      return out_ptr;
    }

    size_t GetSize() const override { return data.size(); }
    void Resize(size_t new_size) override { data.resize(new_size); }
    void Resize(size_t new_size, double default_val) override {
      data.resize(new_size, static_cast<locus_t>(default_val));
    }
    size_t GetNumBytes() const override { return sizeof(locus_t) * GetSize(); }
    void SetSizeRange(size_t _min, size_t _max) override { min_size = _min, max_size = _max; }

    void Randomize(emp::Random & random, size_t pos) override {
      data[pos] = static_cast<locus_t>( random.GetDouble(alphabet_size) );
    }

    // Human-readable (if not easily understandable) shorthand representations.
    // @CAO... needs to be done properly!
    std::string ToString() const override { return "[unknown]"; }
    void FromString(std::string & in) override { emp_error("Cannot read genome from string."); }

    // Potentially more compressed or structured formats for saving/loading genomes.
    // TODO:
    // Archive & Serialize(Archive & archive) = 0;

    // Genome accessors for individual values…
    int ReadInt(size_t index) const override { return static_cast<int>(data[index]); }
    double ReadDouble(size_t index) const override { return static_cast<double>(data[index]); }
    std::byte ReadByte(size_t index) const override { return static_cast<std::byte>(data[index]); }
    bool ReadBit(size_t index) const override { return static_cast<bool>(data[index]); }

    void WriteInt(size_t index, int value) { data[index] = static_cast<locus_t>(value); }
    void WriteDouble(size_t index, double value) { data[index] = static_cast<locus_t>(value); }
    void WriteByte(size_t index, std::byte value) { data[index] = static_cast<locus_t>(value); }
    void WriteBit(size_t index, bool value) { data[index] = static_cast<locus_t>(value); }
 
  };

  template <>
  class TypedGenome<bool> : public Genome {
    // FILL ALL THIS OUT USING emp::BitVector instead!!!
  };

}

#endif
