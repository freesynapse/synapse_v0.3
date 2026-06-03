📖 Complete Manifest Syntax Reference

## 🎯 **Quick Reference Table**

| Property | Type | Manifest Syntax | Default | Valid Range/Values | Notes |
|----------|------|-----------------|---------|-------------------|-------|
| **shader** | string | `shader: pbr_ibl` | (required) | Any loaded shader name | Must exist in shader library |
| **albedo_texture** | string | `albedo_texture: wood_albedo` | "" (none) | Any loaded texture name | Slot 0 (ALBEDO) |
| **normal_texture** | string | `normal_texture: wood_normal` | "" (none) | Any loaded texture name | Slot 1 (NORMAL) |
| **metallic_texture** | string | `metallic_texture: metal_map` | "" (none) | Any loaded texture name | Slot 2 (METALLIC) |
| **roughness_texture** | string | `roughness_texture: rough_map` | "" (none) | Any loaded texture name | Slot 3 (ROUGHNESS) |
| **ao_texture** | string | `ao_texture: ao_map` | "" (none) | Any loaded texture name | Slot 4 (AO) |
| **emissive_texture** | string | `emissive_texture: glow_map` | "" (none) | Any loaded texture name | Slot 5 (EMISSIVE) |
| **use_albedo_map** | bool | `use_albedo_map: true` | `false` | `true`, `false`, `1`, `0`, `yes`, `no` | Enable albedo texture |
| **use_normal_map** | bool | `use_normal_map: true` | `false` | `true`, `false`, `1`, `0`, `yes`, `no` | Enable normal mapping |
| **use_metallic_map** | bool | `use_metallic_map: true` | `false` | `true`, `false`, `1`, `0`, `yes`, `no` | Enable metallic texture |
| **use_roughness_map** | bool | `use_roughness_map: true` | `false` | `true`, `false`, `1`, `0`, `yes`, `no` | Enable roughness texture |
| **use_ao_map** | bool | `use_ao_map: true` | `false` | `true`, `false`, `1`, `0`, `yes`, `no` | Enable AO texture |
| **use_emissive_map** | bool | `use_emissive_map: true` | `false` | `true`, `false`, `1`, `0`, `yes`, `no` | Enable emissive texture |
| **albedo_color** | vec4 | `albedo_color: 1.0 1.0 1.0 1.0` | `1.0 1.0 1.0 1.0` | RGBA [0.0-∞] | Fallback/tint color |
| **emissive_color** | vec4 | `emissive_color: 0.0 1.0 0.5 1.0` | `0.0 0.0 0.0 1.0` | RGBA [0.0-∞] | Glow color |
| **metallic** | float | `metallic: 1.0` | `0.0` | [0.0-1.0] | 0=dielectric, 1=metal |
| **roughness** | float | `roughness: 0.5` | `0.5` | [0.0-1.0] | 0=glossy, 1=rough |
| **ao** | float | `ao: 1.0` | `1.0` | [0.0-1.0] | Ambient occlusion |
| **tiling_factor** | float | `tiling_factor: 2.0` | `1.0` | [0.0-∞] | UV scale multiplier |

---

## 📝 **Type Syntax Details**

### **1. String (Texture/Shader References)**

```yaml
# Format: <property>: <name>
shader: pbr_ibl
albedo_texture: helmet_albedo
normal_texture: helmet_normal
```

**Parsing:**
- Whitespace-trimmed
- Must match an asset name loaded in Pass 1
- Empty string = no texture assigned
- Case-sensitive

**Examples:**
```yaml
albedo_texture: wood_diffuse        # Valid
albedo_texture: Wood_Diffuse        # Different asset (case-sensitive)
albedo_texture:                     # Empty = no texture (optional comma syntax)
# albedo_texture: <omitted>         # Same as empty
```

---

### **2. Boolean (Flags)**

```yaml
# Format: <property>: <value>
use_albedo_map: true
use_normal_map: false
use_metallic_map: 1
use_roughness_map: 0
use_ao_map: yes
use_emissive_map: no
```

**Accepted Values:**

| Input | Result | Notes |
|-------|--------|-------|
| `true` | `true` | Preferred |
| `false` | `false` | Preferred |
| `1` | `true` | Numeric |
| `0` | `false` | Numeric |
| `yes` | `true` | Natural language |
| `no` | `false` | Natural language |
| *anything else* | `false` | **Warning logged** |

**Case Sensitivity:** Case-insensitive (convert to lowercase before comparing)

**Examples:**
```yaml
use_albedo_map: true       # ✅ Preferred
use_albedo_map: TRUE       # ✅ Also valid (convert to lowercase)
use_albedo_map: 1          # ✅ Valid
use_albedo_map: yes        # ✅ Valid
use_albedo_map: enabled    # ⚠️ Defaults to false, warning logged
```

---

### **3. Float (Single Values)**

```yaml
# Format: <property>: <value>
metallic: 1.0
roughness: 0.5
ao: 1.0
tiling_factor: 2.5
```

**Accepted Formats:**

| Input | Parsed Value | Notes |
|-------|--------------|-------|
| `1.0` | `1.0` | Standard decimal |
| `0.5` | `0.5` | Fractional |
| `2` | `2.0` | Integer (converted) |
| `.5` | `0.5` | Leading zero optional |
| `1.5e2` | `150.0` | Scientific notation |
| `invalid` | `0.0` | **Warning logged** |

**Range Validation:**
- Values outside recommended range are accepted (no clamping)
- Engine/shader may clamp internally
- Warnings logged for extreme values (optional)

**Examples:**
```yaml
metallic: 1.0        # ✅ Standard
metallic: 1          # ✅ Converts to 1.0
metallic: 0.75       # ✅ Valid
metallic: 1.5        # ⚠️ Valid but outside [0-1] range
metallic: -0.1       # ⚠️ Valid but negative (may cause issues)
metallic: abc        # ❌ Defaults to 0.0, warning logged
tiling_factor: 2.5   # ✅ Valid UV scale
```

---

### **4. Vec4 (Colors and 4-Component Vectors)**

```yaml
# Format: <property>: <r> <g> <b> <a>
albedo_color: 1.0 1.0 1.0 1.0
emissive_color: 0.0 1.0 0.5 1.0

# Single-value expansion (all components = same value)
albedo_color: 0.5        # Expands to: 0.5 0.5 0.5 0.5

# RGB only (alpha defaults to 1.0) - optional enhancement
# albedo_color: 1.0 0.0 0.0   # Could expand to: 1.0 0.0 0.0 1.0
```

**Parsing Rules:**

| Input Format | Parsed As | Notes |
|--------------|-----------|-------|
| `r g b a` | `vec4(r, g, b, a)` | Standard RGBA |
| `v` | `vec4(v, v, v, v)` | Single value expansion |
| *other* | `vec4(1.0)` | **Warning logged** |

**Component Ranges:**
- Typical: [0.0-1.0] for colors
- HDR emissive colors can exceed 1.0
- No clamping performed

**Examples:**
```yaml
# Standard RGBA
albedo_color: 1.0 0.5 0.25 1.0           # ✅ Orange color
albedo_color: 0.8 0.8 0.8 1.0            # ✅ Light gray

# Single value expansion
albedo_color: 1.0                        # ✅ Expands to: 1.0 1.0 1.0 1.0 (white)
albedo_color: 0.5                        # ✅ Expands to: 0.5 0.5 0.5 0.5 (mid-gray)

# HDR emissive
emissive_color: 2.0 0.5 0.0 1.0          # ✅ Bright orange glow (HDR)
emissive_color: 5.0                      # ✅ Very bright white emission

# Edge cases
albedo_color: 1.0 0.5                    # ❌ Invalid (2 components), defaults to vec4(1.0), warning
albedo_color: invalid                    # ❌ Invalid, defaults to vec4(1.0), warning
```

---

### **5. Vec3 (Future: Positions, Directions)**

Not currently used in PBR materials, but here's the syntax for future reference:

```yaml
# Format: <property>: <x> <y> <z>
uv_offset: 0.5 0.5 0.0
scale: 2.0 2.0 2.0

# Single-value expansion
scale: 2.0           # Expands to: 2.0 2.0 2.0
```

---

## 📋 **Complete Material Examples**

### **Example 1: Minimal (Required Only)**
```yaml
material simple
{
    shader: pbr_ibl
}
```
**Result:** All textures disabled, uses default values.

---

### **Example 2: Textured Wood**
```yaml
material wood_planks
{
    shader: pbr_ibl
    albedo_texture: wood_albedo
    normal_texture: wood_normal
    roughness_texture: wood_roughness
    use_albedo_map: true
    use_normal_map: true
    use_roughness_map: true
}
```

---

### **Example 3: All Boolean Syntaxes**
```yaml
material bool_test
{
    shader: pbr_ibl
    use_albedo_map: true      # Preferred
    use_normal_map: false     # Preferred
    use_metallic_map: 1       # Numeric
    use_roughness_map: 0      # Numeric
    use_ao_map: yes           # Natural
    use_emissive_map: no      # Natural
}
```

---

### **Example 4: All Float Types**
```yaml
material float_test
{
    shader: pbr_ibl
    metallic: 1.0             # Standard
    roughness: 0.5            # Fraction
    ao: 1                     # Integer
    tiling_factor: 2.5        # Greater than 1
}
```

---

### **Example 5: All Vec4 Formats**
```yaml
material color_test
{
    shader: pbr_ibl
    albedo_color: 0.8 0.2 0.2 1.0       # Red-ish RGBA
    emissive_color: 2.0                 # Bright white (HDR)
}
```

---

### **Example 6: Chrome Material (No Textures)**
```yaml
material chrome
{
    shader: pbr_ibl
    albedo_color: 0.95 0.95 1.0 1.0
    metallic: 1.0
    roughness: 0.05
}
```

---

### **Example 7: Fully Specified (All 19 Properties)**
```yaml
material complete
{
    # Required
    shader: pbr_ibl
    
    # 6 texture slots
    albedo_texture: tex_albedo
    normal_texture: tex_normal
    metallic_texture: tex_metallic
    roughness_texture: tex_roughness
    ao_texture: tex_ao
    emissive_texture: tex_emissive
    
    # 6 flags
    use_albedo_map: true
    use_normal_map: true
    use_metallic_map: true
    use_roughness_map: true
    use_ao_map: true
    use_emissive_map: true
    
    # 2 colors
    albedo_color: 1.0 1.0 1.0 1.0
    emissive_color: 0.0 0.0 0.0 1.0
    
    # 4 values
    metallic: 0.0
    roughness: 0.5
    ao: 1.0
    tiling_factor: 1.0
}
```

---

## 🔍 **Parsing Validation Checklist**

When implementing the parser, validate:

✅ **Shader reference exists** (error if missing)  
✅ **Texture references exist** (warning if missing, assign null handle)  
✅ **Boolean values** (warning for invalid, default to false)  
✅ **Float values** (warning for invalid, default to 0.0)  
✅ **Vec4 values** (warning for invalid, default to vec4(1.0))  
⚠️ **Property name typos** (warning for unknown keys)  
⚠️ **Duplicate properties** (last value wins, optional warning)