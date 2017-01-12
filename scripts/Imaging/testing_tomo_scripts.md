## Run Find_Center with a single image and no crop

```python
python tomo_main.py 
--tool=tomopy 
--algorithm=gridrec 
--num-iter=5 
--input-path=/media/matt/Windows/Documents/mantid_workspaces/imaging/RB000888_test_stack_larmor_summed_201510/data_stack_larmor_summed 
--input-path-flat=/media/matt/Windows/Documents/mantid_workspaces/imaging/RB000888_test_stack_larmor_summed_201510/flat_stack_larmor_summed 
--input-path-dark=/media/matt/Windows/Documents/mantid_workspaces/imaging/RB000888_test_stack_larmor_summed_201510/dark_stack_larmor_summed 
--region-of-interest=[0.000000, 0.000000, 511.000000, 511.000000] 
--output=/media/matt/Windows/Documents/mantid_workspaces/imaging/RB000888_test_stack_larmor_summed_201510/processed/reconstruction_TomoPy_gridrec_2016December15_093530_690212000 
--median-filter-size=3 
--cor=255.000000 
--rotation=1 
--max-angle=360.000000 
--circular-mask=0.940000 
--out-img-format=png 
--find-cor
```

> Expected COR: 265.0

## Run Find_Center with full `RB000888_test_stack_larmor_summed_201510` and crop **[36, 227, 219, 510]**

```python
python tomo_main.py 
--tool=tomopy 
--algorithm=gridrec 
--num-iter=5 
--input-path=/media/matt/Windows/Documents/mantid_workspaces/imaging/RB000888_test_stack_larmor_summed_201510/data_full 
--input-path-flat=/media/matt/Windows/Documents/mantid_workspaces/imaging/RB000888_test_stack_larmor_summed_201510/flat_stack_larmor_summed 
--input-path-dark=/media/matt/Windows/Documents/mantid_workspaces/imaging/RB000888_test_stack_larmor_summed_201510/dark_stack_larmor_summed 
--region-of-interest=[36.000000, 227.000000, 511.000000, 511.000000] 
--output=/media/matt/Windows/Documents/mantid_workspaces/imaging/RB000888_test_stack_larmor_summed_201510/processed/reconstruction_TomoPy_gridrec_2016December15_093530_690212000 
--median-filter-size=3 
--cor=255.000000 
--rotation=1 
--max-angle=360.000000 
--circular-mask=0.940000 
--out-img-format=png 
--find-cor
```
> Expected COR: 135.0