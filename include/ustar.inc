%ifndef USTAR_INCLUDED
%define USTAR_INCLUDED

struc tar_t
    tar_file_name: resb 100
    tar_file_mode: resb 8
    tar_owner_uid: resb 8
    tar_group_gid: resb 8
    tar_file_size: resb 12
    tar_lmod_time: resb 12
    tar_hchecksum: resb 8
    tar_type_flag: resb 1
    tar_link_trgt: resb 100
    tar_signature: resb 6
    tar_formatver: resb 2
    tar_user_name: resb 32
    tar_group_name: resb 32
    tar_dev_major: resb 8
    tar_dev_minor: resb 8
    tar_file_prefix: resb 155
endstruc

%endif
