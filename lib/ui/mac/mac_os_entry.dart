import 'package:flutter/material.dart';
import 'package:macos_ui/macos_ui.dart';
import 'package:system_info/ui/mac/home_page.dart';

import 'specification_page.dart';

class MacOsEntry extends StatefulWidget {
  const MacOsEntry({super.key});

  @override
  State<MacOsEntry> createState() => _MacOsEntryState();
}

class _MacOsEntryState extends State<MacOsEntry> {
  int pageIndex = 0;

  @override
  Widget build(BuildContext context) {
    return MacosWindow(
        sidebar: Sidebar(
          minWidth: 150,
          builder: (context, scrollController) {
            return SidebarItems(
              currentIndex: pageIndex,
              scrollController: scrollController,
              itemSize: SidebarItemSize.large,
              onChanged: (i) {
                setState(() => pageIndex = i);
              },
              items: const [
                SidebarItem(
                  leading: Icon(
                    Icons.home_rounded,
                    size: 16,
                  ),
                  label: Text(
                    'Home',
                    style: TextStyle(fontSize: 12),
                  ),
                ),
                SidebarItem(
                  leading: Icon(
                    Icons.computer_rounded,
                    size: 16,
                  ),
                  label: Text(
                    'Specifications',
                    style: TextStyle(fontSize: 12),
                  ),
                ),
              ],
            );
          },
        ),
        backgroundColor: const Color.fromARGB(255, 241, 241, 244),
        child: LayoutBuilder(
          builder: (BuildContext context, BoxConstraints constraints) {
            return pageIndex == 0 ? HomePage() : SpecificationPage();
          },
        ));
  }
}
