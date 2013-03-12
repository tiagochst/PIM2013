/*
**  Header for PLY polygon files.
**  
**  - Greg Turk, March 1994
**  
**  A PLY file contains a single polygonal _object_.
**  
**  An object is composed of lists of _elements_.  Typical elements are
**  vertices, faces, edges and materials.
**  
**  Each type of element for a given object has one or more _properties_
**  associated with the element type.  For instance, a vertex element may
**  have as properties three floating-point values x,y,z and three unsigned
**  chars for red, green and blue.
**  
**  TODO:
**      - Discuss licensing issues, since this is a C++ port of the C 
**      code provided by Greg Turk.
**  
**  ---------------------------------------------------------------
**  
**  Copyright (c) 1994 The Board of Trustees of The Leland Stanford
**  Junior University.  All rights reserved.   
**    
**  Permission to use, copy, modify and distribute this software and its   
**  documentation for any purpose is hereby granted without fee, provided   
**  that the above copyright notice and this permission notice appear in   
**  all copies of this software and that you do not sell the software.   
**    
**  THE SOFTWARE IS PROVIDED "AS IS" AND WITHOUT WARRANTY OF ANY KIND,   
**  EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY   
**  WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.
**/
#pragma once
#include <iostream>
#include <fstream>
#include <string>

#include <stdio.h>
#include <stddef.h>

namespace ply {

    /* PLY routines return status codes. */
    typedef enum PlyStatusCode {
        PLY_OKAY    =    0,     /* ply routine worked okay */
        PLY_ERROR   =   -1,     /* error in ply routine */
    } PlyStatusCode;

    /* Scalar data types supported by PLY format. */
    typedef enum PlyScalarType {
        PLY_START_TYPE  =   0,    
        PLY_CHAR        =   1,    
        PLY_SHORT       =   2,    
        PLY_INT         =   3,    
        PLY_UCHAR       =   4,    
        PLY_USHORT      =   5,    
        PLY_UINT        =   6,    
        PLY_FLOAT       =   7,    
        PLY_DOUBLE      =   8,    
        PLY_END_TYPE    =   9,    
    } PlyScalarType;
    
    /* Description of a PLY property. */
    class PlyProperty {
        /* PLY property types. */
        typedef enum {
            PLY_SCALAR      =   0,
            PLY_LIST        =   1,
        } Type;

        std::string                     m_name;             /* property name */
        int                             m_externalType;     /* file's data type */
        int                             m_internalType;     /* program's data type */
        int                             m_offset;           /* offset bytes of prop in a struct */
    
        Type                            m_type;             /* 1 = list, 0 = scalar */
        int                             m_externalCount;    /* file's count type */
        int                             m_internalCount;    /* program's count type */
        int                             m_offsetCount;      /* offset byte for list count */
    };
    
    /* Description of a PLY element. */
    class PlyElement {
        std::string                     m_name;             /* element name */
        int                             m_num;              /* number of elements in this object */
        int                             m_size;             /* size of element (bytes) or -1 if variable */
        int                             m_nProps;           /* number of properties for this element */
        VECTOR_CLASS<PlyProperty*>      m_props;            /* list of properties in the file */
        char*                           m_storeProp;        /* flags: property wanted by user? */
        int                             m_otherOffset;      /* offset to un-asked-for props, or -1 if none*/
        int                             m_otherSize;        /* size of other_props structure */
    };

    /* Data for one "other" element */
    class PlyOtherElem {
        /* Describes other properties in an element. */
        class Property {
            std::string                     m_name;         /* element name */
            int                             m_size;         /* size of other_props */
            int                             m_nProps;       /* number of properties in other_props */
            VECTOR_CLASS<PlyProperty*>      m_props;        /* list of properties in other_props */
        };

        /* For storing other_props for an other element */
        class Data {
            void*       m_otherProps;
        };    
    
        std::string                     m_elemName;         /* names of other elements */
        int                             m_elemCount;        /* count of instances of each element */
        Data**                          m_otherData;        /* actual property data for the elements */
        Property*                       m_otherProps;       /* description of the property data */
    };

    /* PLY file descriptor. */
    class PlyFile {
    public:
        typedef enum {
            PLY_ASCII      = 1,     /* ascii PLY file */
            PLY_BINARY_BE  = 2,     /* binary PLY file, big endian */
            PLY_BINARY_LE  = 3      /* binary PLY file, little endian */
        } FileType;
        
    private:
        std::fstream                    m_file;             /* file pointer */
        FileType                        m_type;             /* ascii or binary */
        float                           m_version;          /* version number of file */
        int                             m_nElems;           /* number of elements of object */
        VECTOR_CLASS<PlyElement*>       m_elems;            /* list of elements */
        int                             m_nComments;        /* number of comments */
        VECTOR_CLASS<std::string>       m_comments;         /* list of comments */
        int                             m_nObjInfo;         /* number of items of object information */
        VECTOR_CLASS<std::string>       m_objInfo;          /* list of object info items */
        PlyElement*                     m_whichElem;        /* which element we're currently writing */
        VECTOR_CLASS<PlyOtherElem>      m_otherElems;       /* "other" elements from a PLY file */
    
    private:
        PlyFile(const PlyFile& other);
        PlyFile& operator=(PlyFile& other);
    
    public:
        
    };

    /* memory allocation */
    extern char *my_alloc();
    #define myalloc(mem_size) my_alloc((mem_size), __LINE__, __FILE__)

    extern PlyFile *ply_write(FILE *, int, char **, int);
    extern PlyFile *ply_open_for_writing(char *, int, char **, int, float *);
    extern void ply_describe_element(PlyFile *, char *, int, int, PlyProperty *);
    extern void ply_describe_property(PlyFile *, char *, PlyProperty *);
    extern void ply_element_count(PlyFile *, char *, int);
    extern void ply_header_complete(PlyFile *);
    extern void ply_put_element_setup(PlyFile *, char *);
    extern void ply_put_element(PlyFile *, void *);
    extern void ply_put_comment(PlyFile *, char *);
    extern void ply_put_obj_info(PlyFile *, char *);
    extern PlyFile *ply_read(FILE *, int *, char ***);
    extern PlyFile *ply_open_for_reading( char *, int *, char ***, int *, float *);
    extern PlyProperty **ply_get_element_description(PlyFile *, char *, int*, int*);
    extern void ply_get_element_setup( PlyFile *, char *, int, PlyProperty *);
    extern void ply_get_property(PlyFile *, char *, PlyProperty *);
    extern PlyOtherProp *ply_get_other_properties(PlyFile *, char *, int);
    extern ply_get_element(PlyFile *, void *);
    extern char **ply_get_comments(PlyFile *, int *);
    extern char **ply_get_obj_info(PlyFile *, int *);
    extern void ply_close(PlyFile *);
    extern void ply_get_info(PlyFile *, float *, int *);
    extern PlyOtherElems *ply_get_other_element (PlyFile *, char *, int);
    extern void ply_describe_other_elements ( PlyFile *, PlyOtherElems *);
    extern void ply_put_other_elements (PlyFile *);
    extern void ply_free_other_elements (PlyOtherElems *);

    extern int equal_strings(char *, char *);

};
